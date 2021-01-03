// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#define MODTYPE Vchar_detector

//If not search key was defined set it to a carriage return
#ifndef SEARCH_KEY
    #define SEARCH_KEY 0x0D
#endif
#include "Vchar_detector.h"
#include <verilated.h>

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTestBench.hpp"

// Catch Library
#include <catch2/catch.hpp>

void latch_data(SyncTB<MODTYPE>*& tb, int value)
{
    tb->dut->i_data = value;
    tb->dut->i_data_latch = 1; 
    tb->tick(); 
    tb->dut->i_data_latch = 0; 
}

TEST_CASE("Character Detecton","[char-detect][uart]"){
    /*
    Tests basic Functionality of the Character Detector
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("CharDetection.vcd");

    //Be default the output signal should be low
    REQUIRE(!tb->dut->o_char_in_buffer);

    // Putting the requested data on the bus and setting latch high will latch 
    // the data and drive the output high
    latch_data(tb, 0x0D);
    REQUIRE(tb->dut->o_char_in_buffer);
    //After a second tick the rising edge should also go high
    tb->tick();
    REQUIRE(tb->dut->o_char_rising);
    tb->tick();
    //rising edge should only be high for one cycle
    REQUIRE(tb->dut->o_char_in_buffer);
    REQUIRE(!tb->dut->o_char_rising);

    // Putting new data on the buffer deasserts this
    latch_data(tb, 0xFF);
    REQUIRE(!tb->dut->o_char_in_buffer);
}

TEST_CASE("Core Reset","[char-detect][uart]"){
    /*
    When a Reset reset is high during a clock cycle the data in the buffer
    should be cleared
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench

    latch_data(tb, 0x0D);
    REQUIRE(tb->dut->o_char_in_buffer);
    tb->dut->i_reset = 1;
    tb->tick();
    REQUIRE(tb->dut->char_detector__DOT__data_buffer == 0x00);
    REQUIRE(!tb->dut->o_char_in_buffer);
}
