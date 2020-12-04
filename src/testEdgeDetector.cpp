#define MODTYPE Vedge_detector

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "edge_detector/Vedge_detector.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"


TEST_CASE("Rising & Falling Edge Detection","[edge-detect][uart]"){
    /*
    Make sure that a rising edge signal is output for one clock cycle after the rising edge 
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("Rising Edge.vcd");
    tb->dut->i_signal = 1; // set the input signal high
    tb->tick(); //cycle the clock
    //Should detect a rising edge and no falling edge
    REQUIRE(tb->dut->o_rising_w == 1);
    REQUIRE(tb->dut->o_falling_w == 0);
    
    tb->tick(); //cycle the clock
    //Should have neither rising nor falling edge
    REQUIRE(tb->dut->o_rising_w == 0);
    REQUIRE(tb->dut->o_falling_w == 0);

    tb->dut->i_signal = 0;
    tb->tick(); //cycle the clock
    //Should detect a falling edge and no rising edge
    REQUIRE(tb->dut->o_rising_w == 0);
    REQUIRE(tb->dut->o_falling_w == 1);
    
    tb->tick(); //cycle the clock
    //Should detect no change
    REQUIRE(tb->dut->o_rising_w == 0);
    REQUIRE(tb->dut->o_falling_w == 0);
}
