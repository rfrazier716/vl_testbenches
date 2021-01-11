#pragma once

#include <memory> // for smart pointers

//If the clock line wasn't already defined in the parent file define it here
#ifndef CLOCK_LINE
    #define CLOCK_LINE sys_clock
#endif
// If the reset line was not defined already define it here
// define TB_NO_RESET if the dut does not have a reset handler, to avoid a compiler error
#ifndef RST_LINE
    #define RST_LINE    reset
#endif

//TODO: Change the VCD Trace functions to be composition function to separate vl from class
template<class T>
class SyncTB 
{
    const int clock_frequency_; //The system Clock frequency;
    double timestep_ = float(1000/clock_frequency)/2.0; //How long a clock phase is, in ns

    std::unique_ptr<VerilatedVcdC> trace_; //Trace Function
    int tick_count_; //Keeps track of how many clock cycles have occured

    //Virtual functions that get applied before and after the clock edges
    virtual void pre_rising_edge(){} // function called before the rising edge, applied 2ns before the clock edge
    virtual void post_rising_edge(){} // function called after the rising edge, Use to latch input to interfaces
    virtual void pre_falling_edge(){} // function called before the falling edge, applied 2ns before falling edge
    virtual void post_falling_edge(){} // function called after the falling edge, use to latch input to interfaces

public:
    std::unique_ptr<T> dut; // The device being tested
    void tick(); //Execute a clock cycle
#ifndef TB_NO_RESET
    void reset(); //resets the part, assumes there is an input called reset
#endif
    void add_vcd_trace(const char*); //add a trace to the testbench

    SyncTB(int clock_frequency = 100); //Class Constructor
};

template<class T>
void SyncTB<T>::tick(){
    // Execute a clock cycle and update variables
    // you need the eval at start and end, according to zipCPU
    tick_count_++; //Increase how many ticks have occured
	
    dut->CLOCK_LINE = 0;
    dut->eval(); 
    if(trace_) trace_->dump(tick_count_ * 10 - 2); //Dump the signal change before a trace
    
    dut->CLOCK_LINE = 1;
    dut->eval();
    if(trace_) trace_->dump(tick_count_ * 10); //Dump the Signal change on a clock edge

    dut->CLOCK_LINE = 0; 
    dut->eval();
    if(trace_){
        trace_->dump(tick_count_ * 10 + 5); //Dump the negative edge of the clock
        trace_->flush();
    }
}

#ifndef TB_NO_RESET
template<class T>
void SyncTB<T>::reset(){
    // Execute a clock cycle and update variables
    // you need the eval at start and end, according to zipCPU
	dut->reset=1;
    tick();
    dut->reset=0;
}
#endif

template<class T>
void SyncTB<T>::add_vcd_trace(const char* trace_name)
{
    //Adds VCDTrace output to your system
    if(!trace_){ //If the Trace doesn't already exist
        trace_ = new VerilatedVcdC;
		dut->trace(trace_, 00);
		trace_->open(trace_name);
    }
}

template<class T>
SyncTB<T>::SyncTB(int clock_frequency){
    dut = std::make_unique<T>(); // bind a new dut object to the unique_ptr
    tick_count_=0; // set the tick count to zero

    dut->eval(); // Evaluate it so default states update

    Verilated::traceEverOn(true);
    trace_ = NULL; //Null out the pointer
}
