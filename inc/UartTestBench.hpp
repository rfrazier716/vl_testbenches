#pragma once
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <assert.h>

#include "inc/tcp/TCPServer.hpp"
#define PORT 8080
#define SERVER_DEBUG

enum class TxCaptureState {ST_IDLE,ST_DATA_BITS,ST_STOP_BIT};
enum class RxCaptureState {ST_IDLE, ST_DATA_BITS, ST_STOP_BIT};

template<class T>
class UartTestBench{
    const int baudCounter; //how many clock cycles for one UART data frame
    int clockFrequency;
    int clockPeriod;

    //Variables for handling the tx and RX to server transmission
    char lastRxByte = '\0';
    char lastTxByte = '\0';
    char txByteInProgress = '\0'; //a placeholder for the transmit byte as it's being latched

    //Variables for capturing the tx data wire
    TxCaptureState txState = TxCaptureState::ST_IDLE;
    int txCounter;
    int txBitShift = 0;

    void captureTxWire();

    //Variables for pulling bytes from the TCPServer and manipulating them onto the Rx data wire
    RxCaptureState rxState = RxCaptureState::ST_IDLE;
    int rxCounter;
    int rxBitShift = 0;

    void assignRxWire();

    //If we're configuring tx/rx data on the tcp, create a new server object
    TCPServer* server = nullptr; //TCP server to capture IO
    VerilatedVcdC* tbTrace = nullptr; // Trace Function
    int tickCount; // counter for the VCD timing

public:
    T* dut; // The device under test
    UartTestBench(int baudCounter=108, int clockFrequency=50000000); // Class constructor
    ~UartTestBench(); // Class Destructor
    void addVCDTrace(const char* traceName);
    void tick(); // execute one clock cycle of the system
    int attachServer(); // attach the TCP server
    char getLastTxByte(){return lastTxByte;}
    };


// Execute a clock cycle and update variables
// you need the eval at start and end, according to zipCPU
template<class T>
void UartTestBench<T>::tick(){
    tickCount++; //Increase how many ticks have occured
    if(server) assignRxWire(); // if the server is attached pull bytes and mask them onto the Rx Wire
    dut->CLOCK_LINE = 0;
    dut->eval(); 
    if(tbTrace) tbTrace->dump(tickCount*10-2); //Dump the signal change before a trace    
    
    dut->CLOCK_LINE = 1;
    dut->eval();
    captureTxWire(); //If the server is connected capture the tx wire to output on it
    if(tbTrace) tbTrace->dump(tickCount*10); //Dump the Signal change on a clock edge


    dut->CLOCK_LINE = 0; 
    dut->eval();
    if(tbTrace){
        tbTrace->dump(tickCount*10+5); //Dump the negative edge of the clock
        tbTrace->flush();
    }
}

//sets the rx wire according to messages received from the server
template<class T>
void UartTestBench<T>::assignRxWire()
{
    switch(rxState){
        case RxCaptureState::ST_IDLE:
            rxCounter = baudCounter; //reset the rxCounter
            rxBitShift = 0; //the current bit-shift is zero

            dut->i_rx_w = 1; // if there is not data incoming, kake sure the rx wire is pulled high
            server->tick(); // tick the server to capture incoming data
            // if we are able to pull data off the rx buffer go into the data state
            if(server->getBytes((char *)&lastRxByte, 1))
            {
                #ifdef SERVER_DEBUG
                printf("Received: %c from server on tick %d,\r\n", lastRxByte, tickCount);
                #endif
                rxState = RxCaptureState::ST_DATA_BITS; //move into the transmit state
            }
            break;
        
        case RxCaptureState::ST_DATA_BITS:
            rxCounter--; //Decrease the baudCounter
            auto dataPacket = (lastRxByte | 0x100) << 1; //add the appropriate start and stop bits
            dut->i_rx_w = (dataPacket >> rxBitShift) & 0x01; //put the relevant part of data on the Rx Line
            // if the baudCounter zero'd out advance the rxBitShift and reset the counter
            if(rxCounter == 0)
            {
                #ifdef SERVER_DEBUG
                printf("rx wire to %d on tick %d\r\n", dut->i_rx_w, tickCount);
                #endif
                rxCounter = baudCounter; // Reset the Rx Counter
                rxBitShift++; //increment the bit shift
            }
            // if 10 bytes have been received go back into the idle state
            rxState = (rxBitShift == 10) ? RxCaptureState::ST_IDLE : RxCaptureState::ST_DATA_BITS; 
            break;
    }
}

template<class T>
void UartTestBench<T>::captureTxWire()
{
    switch(txState){
        case TxCaptureState::ST_IDLE:
            txCounter = baudCounter*1.5;
            txBitShift = 0;
            // if the output wire is driven low start the capture 
            txState = (dut->o_tx_w == 0) ? TxCaptureState::ST_DATA_BITS : TxCaptureState::ST_IDLE;
            break;
        case TxCaptureState::ST_DATA_BITS:
            txCounter--;//decrement the counter
            //if the counter is zero latch the data bit and go to the next 
            if(txCounter == 0){
                txCounter = baudCounter; // Reset the baud counter
                txByteInProgress |= (dut->o_tx_w << txBitShift); //latch the date being output by the transmitter
                txBitShift++; // increment the bit shift
                //if we've latched 8 bits go to the stop bit
                txState = (txBitShift == 8) ? TxCaptureState::ST_STOP_BIT : TxCaptureState::ST_DATA_BITS;
            }
            break;
        case TxCaptureState::ST_STOP_BIT:
            txCounter--;
            if(txCounter == 0){
                //assert(false); // assert the stop bit is high
                lastTxByte = txByteInProgress; // transfer the last completed byte
                if(server) server->sendMessage((char *)&lastTxByte, 1); //transmit the byte to the client if the server is connected
                #ifdef SERVER_DEBUG
                printf("Transmitted: %c on tick %d,\r\n",txByteInProgress, tickCount);
                #endif
                txByteInProgress = '\0'; //clear the byte in progress 
                txState = TxCaptureState::ST_IDLE; //advance to the idle state
            }
            break;

    }
}


//Attach an initialize a TCPServer object, blocking until a client connects
template<class T>
int UartTestBench<T>::attachServer()
{
    server = new TCPServer(PORT);
    server->connectToClient(); // Block until a client connects
    server->setBlocking(false); //Don't block on read requests   
    server->sendMessage("Connected to UartTestBench Server");
    return 0;
}

template<class T>
void UartTestBench<T>::addVCDTrace(const char* traceName)
{
    //Adds VCDTrace output to your system
    if(!tbTrace){ //If the Trace doesn't already exist
        tbTrace = new VerilatedVcdC;
		dut->trace(tbTrace, 00);
		tbTrace->open(traceName);
    }
}

template<class T>
UartTestBench<T>::UartTestBench(int baudCounter, int clockFrequency): baudCounter(baudCounter), clockFrequency(clockFrequency){
    clockPeriod = 10;//(int)(1000000000/clockFrequency); //the clock period is is rounded, defaults to a 50Mhz clock and 460.8kBaud
    dut = new T; // Assign the model the testbench manipulates
    dut->i_rx_w = 1; // put the input wire high by default 
    dut->eval(); // Evaluate it so default states update

    Verilated::traceEverOn(true); // must be called to use verilated traces
    tickCount = 0;

    txCounter = baudCounter*1.5; // initialize the baud counter to be offset by 1.5 so you latch in the middle of the data
}

template<class T>
UartTestBench<T>::~UartTestBench(){
    //if(tbTrace) delete tbTrace;
    if(server) delete server;
}