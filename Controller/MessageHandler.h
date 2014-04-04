


#ifndef __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__



#include <stdio.h>
#include <string.h>




//
//
//
template <  typename TxQueueType, 
            typename RxQueueType,
            typename TxDeviceType,
            typename CombinationType,
            typename I2CType>
class MessageHandler
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
    MessageHandler(TxQueueType& _txQueue, RxQueueType& _rxQueue, TxDeviceType& _txDevice, ProtocolType& _protocol, I2CType& _i2c) :
        txQueue(_txQueue),
        rxQueue(_rxQueue),
        txDevice(_txDevice),
        messageCount(0),
        protocol(_protocol),
        i2c(_i2c),
        queueTooSmallFlag(false)
    {
    }



    //
    // Called by the protocol with an array of fields (string pointers)
    // in order to process the received message.
    //
    void HandleMessage(char** fields, uint8_t numberOfFields)
    {
    	static char*	knownMessageTypes[]	=
    	{
    		"I2CBusy?",
    		"I2CConfig",
    	};

    	//
    	// Convert type string into an index into the knownMessageTypes.
    	//
    	for(uint8_t i=0; i<NUMBER_OF_ELEMENTS(knownMessageTypes); i++)
    	{
    		if(strcmp((char*)knownMessageTypes[i], (char*)fields[0]) == 0)
    		{
    			//
    			// We have a known message type.
    			//
    			HandleMessage(fields, numberOfFields, i);

    			break;
    		}
    	}

    }

private:

	//
	//
	//
	void HandleMessage(char** fields, uint8_t numberOfFields, uint8_t typeIndex)
	{
		switch(typeIndex)
		{
			case 0:		// I2CBusy?
			{
				//
				// Send a response.
				//
				if(i2c.IsBusy() == true)
				{
			    	protocol.Put("I2CBusy?", 0x00, "Busy", queueTooSmallFlag);
				}
				else
				{
		    		protocol.Put("I2CBusy?", 0x00, "NotBusy", queueTooSmallFlag);
				}
		    	break;
			}

			case 1:		// I2CConfig?
			{
				//
				// Send a response.
				//
		    	protocol.Put("I2CConfigOk!", queueTooSmallFlag);
		    	break;
			}

			default:
			{
				//
				// The knownMessageType does not have a handler.
				//
				DEBUGME;
				break;
			}
		}

	}

    bool            queueTooSmallFlag;
    TxQueueType&    txQueue;
    RxQueueType&    rxQueue;
    TxDeviceType&   txDevice;
    uint8_t         messageCount;
	ProtocolType& 	protocol;
	I2CType&		i2c;

};




#endif


