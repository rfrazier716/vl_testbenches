//A virtual TCP Server that accepts client connections and generates
//appropriate uart signals 

#include "inc/VirtualUart.hpp"

//Class constructor
//assign default initialization
template <class T>
VirtualUart<T>::VirtualUart(int baudTicks, T& uartRxWire, T& uartTxWire):
baudTicks(baudTicks), rxWire(uartRxWire), txWire(uartTxWire), rxBuffer(BUFFER_DEPTH), txBuffer(BUFFER_DEPTH){
    //initialize the state machines to idle
    txState = rxState = UARTState::ST_IDLE;
}

//Tick function advances the uart by one baudtick
template <class T>

void VirtualUart<T>::tick(){
    captureTxWire(); //capture data output on the TX wire
    driveRxWire(); //drive input to the RX wire
}

template <class T>
void VirtualUart<T>::driveRxWire(){
    switch(rxState){
        case UARTState::ST_IDLE:
            rxCounter = baudTicks; //reset the rxCounter
            rxBitShift = 0; //the current bit-shift is zero

            rxWire = 1; // if there is not data incoming, make sure the rx wire is pulled high
            
            // if we are able to pull data off the rx buffer go into the data state
            if(!rxBuffer.empty()){
                lastRxByte = rxBuffer[0]; //pull the latest bit into the rxByte
                rxBuffer.pop_front(); //pop off the buffer
                rxState = UARTState::ST_DATA_BITS; //advance the state
            }
            break;
        
        case UARTState::ST_DATA_BITS:
            rxCounter--; //Decrease the baudCounter
            auto dataPacket = (lastRxByte | 0x100) << 1; //add the appropriate start and stop bits
            rxWire = (dataPacket >> rxBitShift) & 0x01; //put the relevant part of data on the Rx Line
            // if the baudCounter zero'd out advance the rxBitShift and reset the counter
            if(rxCounter == 0)
            {
                rxCounter = baudTicks; // Reset the Rx Counter
                rxBitShift++; //increment the bit shift
            }
            // if 10 bytes have been received go back into the idle state
            rxState = (rxBitShift == 10) ? UARTState::ST_IDLE : UARTState::ST_DATA_BITS; 
            break;
    }
}

template <class T>
void VirtualUart<T>::captureTxWire()
{
    switch(txState){
        case UARTState::ST_IDLE:
            txCounter = baudTicks*1.5;
            txBitShift = 0;
            // if the output wire is driven low start the capture 
            txState = (txWire == 0) ? UARTState::ST_DATA_BITS : UARTState::ST_IDLE;
            break;
        case UARTState::ST_DATA_BITS:
            txCounter--;//decrement the counter
            //if the counter is zero latch the data bit and go to the next 
            if(txCounter == 0){

                txCounter = baudTicks; // Reset the baud counter
                txByteInProgress |= (txWire << txBitShift); //latch the date being output by the transmitter
                txBitShift++; // increment the bit shift
                //if we've latched 8 bits go to the stop bit
                if(txBitShift == 8)
                {
                    txCounter = baudTicks/2; //the stop bit baud counter is halved
                    txState = UARTState::ST_STOP_BIT; //move to the stop bit counter
                }
            }
            break;
        case UARTState::ST_STOP_BIT:
            txCounter--; //decrement teh counter
            if(txCounter == 0){
                lastTxByte = txByteInProgress; // transfer the last completed byte
                txByteInProgress = '\0'; //clear the byte in progress 
                txState = UARTState::ST_IDLE; //advance to the idle state
            }
            break;
    }
}

//add a character to the rx buffer
template <class T>
void VirtualUart<T>::writeRxBuffer(char rxInput){
    rxBuffer.push_back(rxInput);
}