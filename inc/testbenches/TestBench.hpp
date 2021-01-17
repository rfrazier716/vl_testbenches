#ifndef VL_TESTBENCHES_TESTBENCH_H
#define VL_TESTBENCHES_TESTBENCH_H

#include <memory> // for smart
#include "Tracer.hpp"

//If the clock line wasn't already defined in the parent file define it here
#ifndef CLOCK_LINE
    #define CLOCK_LINE i_clk
#endif
// If the reset line was not defined already define it here
// define TB_NO_RESET if the dut does not have a reset handler, to avoid a compiler error
#ifndef RST_LINE
    #define RST_LINE    reset
#endif


namespace tb {

    template<class CHILD, class DUT>
    class TestBench{
    public:
        void add_trace(const char*);
        void tick();
        double get_timestamp(){return timestamp_;}

    private:
        // a Tracer class to write DUT output
        std::unique_ptr<Tracer<DUT>> tracer_;

        // timing related constants
        int tick_count_; //manages how many clock cycles have occured
        const int clock_frequency_; //the clock frequency, in MHz
        const int clock_phase_t_; //the length of a clock phase in ns (single positive/negative phase)
        double timestamp_; //the current timestamp of the testbench, in ns

        std::unique_ptr<DUT> dut_; //the verilated object that's tested
        CHILD* self = static_cast<CHILD *>(this); //used to reference inherited functions

        //make the constructor private so the templating flows properly (only child who is friend can call parent)
        TestBench();
        friend CHILD;
    protected:
    };

    template<class CHILD, typename DUT>
    TestBench<CHILD, DUT>::TestBench(): clock_frequency_(100), clock_phase_t_(500 / clock_frequency_) {
        dut_ = std::make_unique<DUT>(); // construct the DUT
        dut_->eval(); // evaluate the DUT once so initial input/output are set

        tracer_ = std::make_unique<Tracer<DUT>>(*dut_); // initialize the tracer with a reference to the dut

        tick_count_ = 0; //set the tick count to zero
        timestamp_ = 0; //set teh current timestamp to zero
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::add_trace(const char* trace_name) {
        tracer_->add_vcd_trace(trace_name);
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::tick() {
        tick_count_++; // increment the tick count

        timestamp_ += 2*clock_phase_t_; //set timestamp of the clock rising edge occurrence

        // capture input before the clock's rising edge
        self->set_clock(0); //verify the clock is driven low
        dut_->eval();
        tracer_->dump_state(timestamp_-2); //dump the inputs as being applied 2ns before the clock edge

        // drive the clock high and evaluate
        self->set_clock(1);
        dut_->eval();
        tracer_->dump_state(timestamp_); //Logic should instantly update on a rising edge

        // drive the clock low and evaluate (falling edge)
        self->set_clock(0);
        dut_->eval();
        tracer_->dump_state(timestamp_+5); // Write any logic that happens on the falling edge instantly


    }
}
#endif
