#pragma once
#include "verilated.h"
#include "verilated_vcd_c.h"

//If the clock line wasn't already defined in the parent file define it here
#ifndef CLOCK_LINE
    #define CLOCK_LINE sys_clock
#endif

template<class T>
class SyncTB 
{
    bool traceOutput; //Whether or not to trace the output to a gtkwave file
    const int clockFrequency; //The system Clock frequency;
    double timeStep; //How long a timestep is, in nanoseconds

    VerilatedVcdC* tbTrace; //Trace Function
    int tickCount; //Keeps track of how many clock cycles have occured

public:
    T* dut; // The device being tested
    void tick(); //Execute a clock cycle
    void reset(); //resets the part, assumes there is an input called reset
    void addVCDTrace(const char*);

    SyncTB(int clockFrequency, bool traceOutput); //Class Constructor
};

template<class T>
void SyncTB<T>::tick(){
    // Execute a clock cycle and update variables
    // you need the eval at start and end, according to zipCPU
    tickCount++; //Increase how many ticks have occured
	
    dut->CLOCK_LINE = 0;
    dut->eval(); 
    if(tbTrace) tbTrace->dump(tickCount*10-2); //Dump the signal change before a trace    
    
    dut->CLOCK_LINE = 1;
    dut->eval();
    if(tbTrace) tbTrace->dump(tickCount*10); //Dump the Signal change on a clock edge

    dut->CLOCK_LINE = 0; 
    dut->eval();
    if(tbTrace){
        tbTrace->dump(tickCount*10+5); //Dump the negative edge of the clock
        tbTrace->flush();
    }
}

template<class T>
void SyncTB<T>::reset(){
    // Execute a clock cycle and update variables
    // you need the eval at start and end, according to zipCPU
	dut->reset=1;
    tick();
    dut->reset=0;
}

template<class T>
void SyncTB<T>::addVCDTrace(const char* traceName)
{
    //Adds VCDTrace output to your system
    if(!tbTrace){ //If the Trace doesn't already exist
        tbTrace = new VerilatedVcdC;
		dut->trace(tbTrace, 00);
		tbTrace->open(traceName);
    }
}

template<class T>
SyncTB<T>::SyncTB(int fClock, bool trace): traceOutput(trace), clockFrequency(fClock){
    dut = new T; // Assign the model the testbench manipulates
    tickCount=0; // set the tick count to zero
    Verilated::traceEverOn(true);
    tbTrace = NULL; //Null out the pointer
}
