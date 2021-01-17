//
// Created by rfraz on 1/14/2021.
//

#ifndef VL_TESTBENCHES_TRACER_HPP
#define VL_TESTBENCHES_TRACER_HPP

#include <memory> // for Smart Pointers

//Verilator Headers
#include "verilated.h"
#include "verilated_vcd_c.h"

//Tracer class acts as in interface to a VCD Trace
template <class T>
class Tracer{
public:
    Tracer(T& dut); // Class Constructor
    ~Tracer(); //Class Destructor
    void add_vcd_trace(const char* trace_name="Trace"); //initialize a vcd trace

    // Functions to write data to the trace
    void dump_state(const double timestamp); //write the current state of the DUT to the file
    void flush_buffer(); //flush the vcd buffer to a file

protected:

private:
    // Internal variables
    std::unique_ptr<VerilatedVcdC> trace_; //Pointer to trace stream
    T& dut_; //Reference to the DUT, lifetime must be insured?

    void initialize_trace(); //set-up the vcd trace
};

template <class T>
Tracer<T>::Tracer(T& dut_ref): dut_(dut_ref){
    initialize_trace();
}

template<class T>
Tracer<T>::~Tracer() {
    //If a trace was opened, flush pending output and close it
    if(trace_){
        trace_->flush();
        trace_->close();
    }
}

template<class T>
void Tracer<T>::add_vcd_trace(const char* trace_name) {
    // if the trace is a null pointer initialize it
    if(!trace_){
        trace_ = std::make_unique<VerilatedVcdC>();
        dut_.trace(trace_.get(), 00); //bind the DUT to dump its data to the trace
        trace_->open(trace_name); //Open the trace
    }
}

template<class T>
void Tracer<T>::dump_state(const double timestamp) {
    if(trace_){
        trace_->dump(timestamp);
    }
}

template<class T>
void Tracer<T>::flush_buffer() {
    if(trace_){
        trace_->flush();
    }
}

template<class T>
void Tracer<T>::initialize_trace() {
    Verilated::traceEverOn(true);
    trace_ = NULL; //Null out the pointer
}

#endif //VL_TESTBENCHES_TRACER_HPP
