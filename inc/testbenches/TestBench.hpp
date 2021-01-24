#ifndef VL_TESTBENCHES_TESTBENCH_H
#define VL_TESTBENCHES_TESTBENCH_H

#include <memory> // for smart
#include "Tracer.hpp"
#include "interfaces/VirtualInterface.hpp"


namespace tb {

    //The accessor class is a common friend class so that TestBench can access protected functions in inherited classes
    struct Accessor{
        template<typename CHILD, typename DUT> friend class TestBench;
    private:
        template<typename T>
        static void do_pre_rising(T& derived){
            derived->pre_rising_edge();
        }
        template<typename T>
        static void do_post_rising(T& derived){
            derived->post_rising_edge();
        }
        template<typename T>
        static void do_post_falling(T& derived){
            derived->post_falling_edge();
        }
    };

    template<class CHILD, class DUT>
    class TestBench{
    public:
        void add_trace(const char*);
        void tick();
        double get_timestamp(){return timestamp_;}

    private:
        std::vector<interface::BaseInterface *> interfaces_;
        void execute_interface_pre_tick();
        void execute_interface_post_tick();
        // a Tracer class to write DUT output
        std::unique_ptr<Tracer<DUT>> tracer_;

        // timing related constants
        int tick_count_;
        const int clock_frequency_; //the clock frequency, in MHz
        const int clock_phase_t_; //the length of a clock phase in ns (single positive/negative phase)
        double timestamp_; //the current timestamp of the testbench, in ns

        CHILD* self = static_cast<CHILD *>(this); //used to reference inherited functions

        //make the constructor private so the templating flows properly (only child who is friend can call parent)
        TestBench();
        friend CHILD;

    protected:
        void attach(interface::BaseInterface* interface); // attach an interface to the testbench
        // The dut is protected so inherited testbenches can directly access it
        std::unique_ptr<DUT> dut_; //the verilated object that's tested

        // These protected functions can be overridden by the child to add functionality during a tick function
        void pre_rising_edge(){};
        void post_rising_edge(){};
        void post_falling_edge(){};

    };

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::execute_interface_pre_tick() {
        for(auto interface: interfaces_){
            interface->pre_tick();
        }
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::execute_interface_post_tick() {
        for(auto interface: interfaces_){
            interface->post_tick();
        }
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::attach(interface::BaseInterface *interface) {
        interfaces_.push_back(interface);
    }

    template<class CHILD, typename DUT>
    TestBench<CHILD, DUT>::TestBench(): clock_frequency_(100), clock_phase_t_(500 / clock_frequency_) {
        dut_ = std::make_unique<DUT>(); // construct the DUT
        dut_->eval(); // evaluate the DUT once so initial input/output are set

        tracer_ = std::make_unique<Tracer<DUT>>(*dut_); // initialize the tracer with a reference to the dut

        timestamp_ = 0; //set teh current timestamp to zero
        tick_count_ = 0; // tick count should be zero
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::add_trace(const char* trace_name) {
        tracer_->add_vcd_trace(trace_name);
    }

    template<class CHILD, class DUT>
    void TestBench<CHILD, DUT>::tick() {
        //increment the tick count
        tick_count_++;

        // capture input before the clock's rising edge
        timestamp_ = 2*tick_count_*clock_phase_t_ -2;
        Accessor::do_pre_rising(self);
        execute_interface_pre_tick(); // run the pre-tick operations for all interfaces
        self->set_clock(0); //verify the clock is driven low
        dut_->eval();
        tracer_->dump_state(timestamp_); //dump the inputs as being applied 2ns before the clock edge

        // drive the clock high and evaluate
        timestamp_+=2;
        self->set_clock(1);
        dut_->eval();
        tracer_->dump_state(timestamp_); //Logic should instantly update on a rising edge
        Accessor::do_post_rising(self);
        execute_interface_post_tick();  // run the post tick operations for all interfaces

        // drive the clock low and evaluate (falling edge)
        timestamp_+=clock_phase_t_;
        self->set_clock(0);
        dut_->eval();
        tracer_->dump_state(timestamp_); // Write any logic that happens on the falling edge instantly
        Accessor::do_post_falling(self);
    }
}
#endif
