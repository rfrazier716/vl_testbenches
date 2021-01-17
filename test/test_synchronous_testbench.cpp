//
// Created by rfraz on 1/10/2021.
//


// Catch Library
#include <memory>
#include <catch2/catch.hpp>
#include "testbenches/TestBench.hpp"
#include "Vd_flip_flop.h"
#include <verilated.h>


class FFTestBench: public tb::TestBench<FFTestBench,Vd_flip_flop>{
public:
    void set_clock(uint8_t clock_value){this->dut_->i_clk = clock_value;}
    void set_data_in(uint8_t value){ this->dut_->i_data_in = value;}
    CData get_q_out(){ return this->dut_->o_q_out;}
    CData get_nq_out(){ return this->dut_->o_nq_out;}
};


TEST_CASE("Class Initialization","[testbench]"){
    auto tb = std::make_unique<FFTestBench>();
    REQUIRE(tb->get_q_out()==0);
    REQUIRE(tb->get_nq_out()==1);
}

TEST_CASE("Tick Functional","[testbench]"){
    /*
     * The Tick function should latch the data and update the output, as well as update the timestamp
     */
    auto tb = std::make_unique<FFTestBench>();
    tb->set_data_in(1);
    tb->tick();
    REQUIRE(tb->get_q_out()==1);
    REQUIRE(tb->get_nq_out()==0);
    REQUIRE(tb->get_timestamp() == 10);

    // Verify that the timestamp is updating to be the negative clock phase on every tick
    bool timestamp_updating = true;
    int expected_time_gap = 10;
    for(int j=0;j<10;j++) {
        auto previous_timestamp = tb->get_timestamp();
        tb->tick();
        timestamp_updating &= (tb->get_timestamp()-previous_timestamp) == expected_time_gap;
    }
    REQUIRE(timestamp_updating);
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