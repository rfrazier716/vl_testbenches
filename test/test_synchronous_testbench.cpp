//
// Created by rfraz on 1/10/2021.
//


// Catch Library
#include <memory>
#include <catch2/catch.hpp>
#include "testbenches/TestBench.hpp"
#include "Vd_flip_flop.h"
#include <verilated.h>

namespace {
    class TestInterface : public tb::interface::BaseInterface {
    public:
        int pre_tick_count_;
        int post_tick_count_;

        void pre_tick() override {
            pre_tick_count_++;
        }

        void post_tick() override {
            post_tick_count_++;
        }

        TestInterface() {
            pre_tick_count_ = 0;
            post_tick_count_ = 0;
        }
    };


    class FFTestBench : public tb::TestBench<FFTestBench, Vd_flip_flop> {
    public:
        double pre_rising_timestamp_;
        double post_rising_timestamp_;
        double post_falling_timestamp_;

        // Declare two interfaces to attach to the testbench
        TestInterface interface_a_;
        TestInterface interface_b_;

        void set_clock(uint8_t clock_value) { this->dut_->i_clk = clock_value; }

        void set_data_in(uint8_t value) { this->dut_->i_data_in = value; }

        void reset() {
            this->dut_->i_rst = 1;
            this->tick();
            this->dut_->i_rst = 0;
        }

        CData get_q_out() { return this->dut_->o_q_out; }

        CData get_nq_out() { return this->dut_->o_nq_out; }

        FFTestBench() {
            pre_rising_timestamp_ = 0;
            post_rising_timestamp_ = 0;
            post_falling_timestamp_ = 0;

            //Attach the two interfaces to the system
            attach(&interface_a_);
            attach(&interface_b_);
        }

    protected:
        void pre_rising_edge() { pre_rising_timestamp_ = this->timestamp_; }

        void post_rising_edge() { post_rising_timestamp_ = this->timestamp_; }

        void post_falling_edge() { post_falling_timestamp_ = this->timestamp_; }

        friend class tb::Accessor; // must be used to override protected functions
    };
}




TEST_CASE("Class Initialization","[testbench]"){
    auto tb = std::make_unique<FFTestBench>();
    REQUIRE(tb->get_q_out()==0);
    REQUIRE(tb->get_nq_out()==1);
}

TEST_CASE("Tick Functional","[testbench]"){
    /*
     * The Tick function should latch the data and update the output, as well as update the timestamp
     */
    auto clock_period = 10;
    auto tb = std::make_unique<FFTestBench>();
    tb->set_data_in(1);
    tb->tick();
    REQUIRE(tb->get_q_out()==1);
    REQUIRE(tb->get_nq_out()==0);
    REQUIRE(tb->get_timestamp() == clock_period * 3 / 2); // after a tick the last timestamp was the falling edge

    // Verify that the timestamp is updating to be the negative clock phase on every tick
    bool timestamp_updating = true;
    for(int j=0;j<10;j++) {
        auto previous_timestamp = tb->get_timestamp();
        tb->tick();
        timestamp_updating &= (tb->get_timestamp()-previous_timestamp) == clock_period;
    }
    REQUIRE(timestamp_updating);
}

TEST_CASE("Child TestBench can access dut","[testbench]"){
    /*
     * the reset function is defined by a child of TestBench, being able to write to it means the protected dut_
     * is accessible from the child
     */
    auto tb = std::make_unique<FFTestBench>();
    tb->set_data_in(1);
    tb->tick();
    REQUIRE(tb->get_q_out()==1);
    tb->reset();
    REQUIRE(tb->get_q_out()==0);
}

TEST_CASE("Calling Pre and post tick functions","[testbench]"){
    /*
     *  Verifies functionality of the accessor class and that child classes can override the pre
     *  and post tick functions.
     */
    auto tb = std::make_unique<FFTestBench>();
    //with default testbench frequency frequency we expect the timestamps to be at 8,10, and 15 ns
    tb->tick();

    auto expected_ts_at_start = 8.0;
    REQUIRE(tb->pre_rising_timestamp_ == Approx(expected_ts_at_start));
    REQUIRE(tb->post_rising_timestamp_ == Approx(expected_ts_at_start+2));
    REQUIRE(tb->post_falling_timestamp_ == Approx(expected_ts_at_start+7));
}

TEST_CASE("writing output to a VCD Trace","[testbench]"){
    /*
     * Test is not automated but created a vcd trace to examine
     */
    auto tb = std::make_unique<FFTestBench>();
    tb->add_trace("Test_Trace.vcd");
    for(int j=0;j<10;j++) {
        tb->set_data_in(!tb->get_q_out());
        tb->tick();
    }
    REQUIRE(true);
}

TEST_CASE("Executing Attached Interfaces","[testbench]"){
    /*
     *  all interfaces that are attached should have pre and post-tick functions executed once per tick
     */
    auto tb = std::make_unique<FFTestBench>();
    for(int j=0;j<10;j++){
        tb->tick();
        //verify interface A's pre and post tick ran
        REQUIRE(tb->interface_a_.pre_tick_count_==j+1);
        REQUIRE(tb->interface_a_.post_tick_count_==j+1);

        //verify interface B's pre and post tick ran
        REQUIRE(tb->interface_b_.pre_tick_count_==j+1);
        REQUIRE(tb->interface_b_.post_tick_count_==j+1);

    }

}



