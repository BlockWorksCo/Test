




import serial
import re
import time
import threading
import Queue



def ReadNextMessage(serialPort, q):
    """
    Read the messages from the InterfaceTest board serial port into a Queue.
    NOTE: Called on a separate thread.
    """
    while True:

        text        = serialPort.readline()
        #print(text)
        matches    = re.compile('>(.*),(.*),(.*),(.*)\r').findall(text)
        #print(matches)

        for messageCount,messageType,targetAddress,payload in matches:
            msg     = {
                        'SequenceNo' : messageCount,
                        'Type'       : messageType,
                        'Target'     : targetAddress,
                        'Payload'    : payload
                       }


            q.put(msg)




def ListFromQueue(q):
    """
    Read all elements currently in the q into a list.
    """
    done        = False
    elementList = []
    while done == False:

        try:
            msg = q.get(False)            
            elementList.append(msg)
        except:
            done = True

    return elementList



def EmptyQueue(q):
    """
    Remove all elements from the queue.
    NOTE: This doesn't guarantee that the queue is empty... relying on that would be a race condition.
    """

    done        = False
    while done == False:

        try:
            msg = q.get(False)            
        except:
            done = True





def TestMasterSingleByteWrite(serialPort):
    """
    1.3. Master Single byte write (atomic/register based)
    Tests ability to write single bytes to a slave device using the atomic (register-based) method.
    """

    #
    # Assert that the bus is free.
    #
    EmptyQueue(q)
    serialPort.write('>00,I2CBusy?,slave,0x12\n')
    time.sleep(1.0)
    messages    = ListFromQueue(q)
    matches     = [message for message in messages if message['Type'] == 'I2CBusy?']
    print(matches)
    assert(matches != [])
    assert(matches[0]['Payload'] == 'NotBusy')

    #
    # Assert that there are no outstanding errors.
    #
    #uci.GetErrorList()

    #
    # Place DUT into master mode.
    #
    #uci.SetMasterMode()

    #
    # Place InterfaceTest board into slave mode.
    #
    serialPort.write('>00,I2CConfig!,slave,0x12\n')

    #
    # Cause DUT to perform a master mode write to slave 0x12 of byte string [0xab]
    #
    #uci.PerformMasterModeWrite('a')

    #
    # Wait for the transaction to occur.
    #
    EmptyQueue(q)
    time.sleep(1.0)

    #
    # Assert that harness received only byte string [0xab]
    #
    messages    = ListFromQueue(q)
    matches     = [message for message in messages if message['Type'] == 'SlWr!']
    print(matches)
    assert(matches != [])

    #
    # Assert that bus is free.
    #
    serialPort.write('>00,I2CBusy?,slave,0x12\n')
    time.sleep(1.0)
    messages    = ListFromQueue(q)
    matches     = [message for message in messages if message['Type'] == 'I2CBusy?']
    print(matches)
    assert(matches != [])
    assert(matches[0]['Payload'] == 'NotBusy')

    #
    # Assert that no exceptions were raised on DUT (error messages from control channel).
    #
    #uci.GetErrorList()

    #
    # Assert that no incorrect status received on harness (return from aa_i2c_slave_read())
    #
    #no exceptions so far indicates Ok.













if __name__ == '__main__':
    """
    """

    #
    # Setup:
    # Start the thread reading messages from the InterfaceTest board.
    #
    serialPort = serial.Serial(3, 115200, timeout=None, parity=serial.PARITY_NONE, rtscts=0)
    q = Queue.Queue()
    t = threading.Thread(target=ReadNextMessage, args = (serialPort,q))
    t.daemon    = True
    t.start()


    #
    # Run the test.
    #
    TestMasterSingleByteWrite(serialPort)


    #
    # Clean up.
    #





