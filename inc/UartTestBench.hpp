#pragma once
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <assert.h>

#include "inc/tcp/TCPServer.hpp"
#define PORT 8080

enum class TxCaptureState {ST_IDLE,ST_DATA_BITS,ST_STOP_BIT};

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
    //update RX line based on server port
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
                printf("Transmitted: %c on tick %d,\r\n",txByteInProgress, tickCount);
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