







#include <stdint.h>
#include "Queue.h"
#include "UART.h"
#include "I2C.h"
#include "InterfaceBoardProtocol.h"
#include "MessageHandler.h"



#define I2C_SLAVE_ADDRESS       (0x40)



//
// USART1 configuration.
//
typedef Queue<char, 128, uint8_t>                               UARTQueueType;
typedef UART<USART1_BASE, UARTQueueType, UARTQueueType, 115200>             UARTType;
UARTQueueType                                                               usart1ReceiveQ;
UARTQueueType                                                               usart1TransmitQ;
UARTType                                                                    uart1(usart1TransmitQ, usart1ReceiveQ);

//
// I2C1 configuration.
//
typedef Queue<I2CCommand, 64, uint8_t>                                              I2CQueueType;
typedef I2C<I2C1_BASE, I2CQueueType, I2CQueueType, 100000, I2C_SLAVE_ADDRESS<<1>    I2CType;
I2CQueueType                                                                i2c1ReceiveQ;
I2CQueueType                                                                i2c1TransmitQ;
I2CType                                                                     i2c1(i2c1TransmitQ, i2c1ReceiveQ);

//
// MessageHandler & HostProtocol configuration.
// This is a combination type to represent all the circularly-dependent template parameters.
// MessageHandler & HostProtocol are dependent on each other through their parameterised types, therefore we define the
// types encapsulated in a shared CombinationType and break them out within the individual classes.
//
struct MessageCombo
{
    typedef MessageHandler<UARTQueueType, UARTQueueType, UARTType, MessageCombo, I2CType>   MessageHandlerType;
    typedef InterfaceBoardProtocol<UARTQueueType, UARTQueueType, UARTType, MessageCombo>    HostProtocolType;
};

extern MessageCombo::HostProtocolType                                                     hostProtocol; // Forward declaration to allow for circular dependencies.
MessageCombo::MessageHandlerType                                                          messageHandler(usart1TransmitQ, usart1ReceiveQ, uart1, hostProtocol, i2c1);
MessageCombo::HostProtocolType                                                            hostProtocol(usart1TransmitQ, usart1ReceiveQ, uart1, messageHandler);















//
// Hook up the USART1 IRQ to the ISR.
//
extern "C" void USART1_IRQHandler(void)
{
    uart1.ISR();
}


//
// Hook up the I2C1 Event IRQ to the ISR.
//
extern "C" void I2C1_EV_IRQHandler(void)
{
    i2c1.EventISR();
}

//
// Hook up the I2C1 Error IRQ to the ISR.
//
extern "C" void I2C1_ER_IRQHandler(void)
{
    i2c1.ErrorISR();
}



















//
//
//
extern "C" int AppMain(void)
{
    bool queueTooSmallFlag  = false;

    //
    // Sign on....
    //
    hostProtocol.Put("UUUU", queueTooSmallFlag);
    hostProtocol.Put("\r\n Interface Test Board \r\n", queueTooSmallFlag);

    //
    // Forever....
    //
    while(true)
    {
        //
        // Process UART receiveQ.
        //
        if(usart1ReceiveQ.IsEmpty() == false)
        {
            //
            // We have data from the host, process the protocol queue.
            //
            hostProtocol.ProcessQueue();
        }           

        //
        // Process I2C receiveQ.
        //
        {
            bool            dataAvailableFlag   = false;        
            I2CCommand      transaction         = i2c1ReceiveQ.Get(dataAvailableFlag);

            if(dataAvailableFlag == true)
            {
                //
                // Transmit the transaction data back to the PC.
                //
                hostProtocol.Put(transaction, queueTooSmallFlag);
            }           
        }

    }

    return 0;
}


