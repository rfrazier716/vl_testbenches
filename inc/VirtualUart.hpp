#pragma once // only load into memory once
#include <boost/circular_buffer.hpp>
#define BUFFER_DEPTH 1024

//An enum to control the uart state machines
enum class UARTState {ST_IDLE,ST_DATA_BITS,ST_STOP_BIT};

template <class T>
class VirtualUart{
    const int baudTicks; //How many ticks per baud clock
    T& rxWire; // the receive wire -- the server drive this wire based on input
    T& txWire; // the tx wire -- server samples this wire to generate output
    char lastTxByte; // the last byte that was transmitted by the server
    char lastRxByte; // the last byt that was received by the server
    boost::circular_buffer<char> rxBuffer; //buffer to put input to assert to the rx wire
    boost::circular_buffer<char> txBuffer; //buffer to put data that's being transmitted out
    
    //variables for the tx and rx state machines
    UARTState txState; // the current state of the txCapture state machine
    int txCounter; //counter for state transition
    int txBitShift; //shifts the incoming data to the correct bit
    char txByteInProgress='\0'; //The current tx byte that's being built

    UARTState rxState; // the current state of the rxTranslator state machine
    int rxCounter; // counter for state transition
    int rxBitShift; //Receiver bit shift

public:
    VirtualUart(int baudTicks, T& uartRxWire, T& uartTxWire);
    void tick(); // the tick function advances the clock
    void captureTxWire(); //function to pull data off the tx wire
    void driveRxWire(); //function to drive the rxWire

    
    char getLastTxByte(){return lastTxByte;}
    char getLastRxByte(){return lastRxByte;}
    UARTState getTxState(){return txState;}
    UARTState getRxState(){return rxState;}
    int getTxValue(){return txWire;} //current value of the tx wire
    int getRxValue(){return rxWire;} //current value of the rx wire
    void writeRxBuffer(char rxInput); //add a character to the rx buffer
};

#include "VirtualUart.tpp" //Include the Template Implementation