


#ifndef __INTERFACEBOARDPROTOCOL_H__
#define __INTERFACEBOARDPROTOCOL_H__



#include "Utilities.h"
#include "I2C.h"








//
//
//
template <  typename TxQueueType, 
            typename RxQueueType,
            typename TxDeviceType,
            typename CombinationType
            >
class InterfaceBoardProtocol
{
    //
    // Break out the CombinationTypes for this class.
    //
    typedef typename CombinationType::MessageHandlerType    MessageHandlerType;
    typedef typename CombinationType::HostProtocolType      ProtocolType;

public:


    //
    //
    //
    InterfaceBoardProtocol(TxQueueType& _txQueue, RxQueueType& _rxQueue, TxDeviceType& _txDevice, MessageHandlerType& _messageHandler) :
        txQueue(_txQueue),
        rxQueue(_rxQueue),
        txDevice(_txDevice),
        messageCount(0),
        messageHandler(_messageHandler),
        queueTooSmallFlag(false)
    {

    }


    //
    //
    //
    void Put(uint8_t byte, bool& queueTooSmallFlag)
    {
        uint8_t     highNibble  = (byte >> 4) & 0xf;
        uint8_t     lowNibble   = (byte) & 0xf;
        Put(NIBBLE_TO_HEX(highNibble), queueTooSmallFlag);
        Put(NIBBLE_TO_HEX(lowNibble), queueTooSmallFlag);
    }



    //
    //
    //
    void Put(char c, bool& queueTooSmallFlag)
    {
        //
        // Put the byte-to-transmit in the queue.
        //
        txQueue.Put(c, queueTooSmallFlag);
        txDevice.ProcessQueue();
    }


    //
    //
    //
    void Put(const char *s, bool& queueTooSmallFlag)
    {
        //
        // Start-Of-Message
        //
        Put('>', queueTooSmallFlag);

        //
        // Sequence Number.
        //
        Put(messageCount, queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // Type.
        //
        RawPut("Text", queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // String payload.
        //
        while( (*s) != 0)
        {
            Put(*s, queueTooSmallFlag);
            s++;
        }

        //
        // Missing field.
        //
        Put(' ', queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // End-Of-Message.
        //
        Put('\r', queueTooSmallFlag);
        Put('\n', queueTooSmallFlag);
    }


    //
    //
    //
    void Put(const char *type, uint8_t targetAddress, const char* payload, bool& queueTooSmallFlag)
    {
        //
        // Start-Of-Message
        //
        Put('>', queueTooSmallFlag);

        //
        // Sequence Number.
        //
        Put(messageCount, queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // Type.
        //
        while( (*type) != 0)
        {
            Put(*type, queueTooSmallFlag);
            type++;
        }

        Put(',', queueTooSmallFlag);

        //
        // 'Target' field
        //
        Put(targetAddress, queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // String payload.
        //
        while( (*payload) != 0)
        {
            Put(*payload, queueTooSmallFlag);
            payload++;
        }

        //
        // End-Of-Message.
        //
        Put('\r', queueTooSmallFlag);
        Put('\n', queueTooSmallFlag);
    }




    //
    //
    //
    void Put(I2CCommand& transaction, bool& queueTooSmallFlag)
    {
        //
        // Start-Of-Message
        //
        RawPut(">", queueTooSmallFlag);

        //
        // Sequence Number.
        //
        Put(messageCount, queueTooSmallFlag);
        Put(',', queueTooSmallFlag);

        //
        // Type.
        //
        if(transaction.type == i2cSlaveWrite)
        {
            RawPut("SlWr!", queueTooSmallFlag);
        }
        else if(transaction.type == i2cSlaveRead)
        {
            RawPut("SlRd!", queueTooSmallFlag);
        }
        else if(transaction.type == i2cError)
        {
            RawPut("BErr!", queueTooSmallFlag);
        }
        Put(',', queueTooSmallFlag);

        //
        // I2C target address.
        //
        {
            uint8_t     address8bit     = transaction.target >> 1;      // TODO: 10-bit addresses.
            Put(address8bit, queueTooSmallFlag);
        }
        Put(',', queueTooSmallFlag);

        //
        // I2C payload.
        //
        for(int i=0; i<transaction.numberOfBytes; i++)
        {
            Put(transaction.payload[i], queueTooSmallFlag);
        }

        //
        // End-Of-Message.
        //
        Put('\r', queueTooSmallFlag);
        Put('\n', queueTooSmallFlag);

        //
        // Keep track of how many we have sent for sequencing reasons.
        //
        messageCount++;
    }


    //
    // Process the master command queue.
    // TODO: Use escaping for the control characters.
    //
    bool ProcessQueue()
    {
        bool            dataAvailableFlag   = false;
        char            character           = rxQueue.Get(dataAvailableFlag);
        static uint8_t  position            = 0;
        static uint8_t  fieldNumber         = 0;
        static char     message[256];
        static char*    fields[16];


        if(dataAvailableFlag == true)
        {
            switch(character)
            {
                case '\r':
                case '\n':
                {
                    //
                    // End-Of-Message....
                    // Check the message and handle it.
                    //
                    if(position > 0)
                    {
                        //
                        // Handle message.
                        //
                        messageHandler.HandleMessage(&fields[1], fieldNumber);
                    }
                    position    = 0;

                    break;
                }

                case '>':
                {
                    //
                    // Start-Of-Message
                    // Reset the fields & position info.
                    //
                    position    = 0;
                    fieldNumber = 0;
                    fields[fieldNumber] = &message[position];

                    break;
                }

                case ',':
                {
                    //
                    // Next field.
                    // Mark the end of the field in the buffer and mark the start of the next.
                    //
                    message[position] = 0;
                    position = (position + 1) % sizeof(message);

                    fieldNumber++;
                    if(fieldNumber < NUMBER_OF_ELEMENTS(fields))
                    {
                        fields[fieldNumber] = &message[position];
                    }
                    else
                    {
                        //
                        // Too many fields.
                        //
                        DEBUGME;
                    }
                    break;
                }

                default:
                {
                    //
                    // Field content.
                    // Just store it for later use by the MessageHandler.
                    //
                    if(position < NUMBER_OF_ELEMENTS(message))
                    {
                        message[position] = character;
                        position++;
                    }
                    else
                    {
                        //
                        // Message too long.
                        //
                        DEBUGME;
                    }
    
                    break;
                }
            }
        }

        return false;
    }



private:


    //
    //
    //
    void RawPut(const char *s, bool& queueTooSmallFlag)
    {
        //
        // String payload.
        //
        while( (*s) != 0)
        {
            Put(*s, queueTooSmallFlag);
            s++;
        }
    }


    //
    //
    //
    char NIBBLE_TO_HEX(uint8_t nibble)
    {
        return nibble<10 ? nibble+'0' : (nibble-10)+'A';
    }




    bool                queueTooSmallFlag;
    TxQueueType&        txQueue;
    RxQueueType&        rxQueue;
    TxDeviceType&       txDevice;
    MessageHandlerType& messageHandler;
    uint8_t             messageCount;


};



#endif

