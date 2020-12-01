#define MODTYPE Vlinefeed_detector

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "linefeed_detector/Vlinefeed_detector.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"


TEST_CASE("Linefeed Detection","[fifo]"){
    /*
    Make sure that a rising edge signal is output for one clock cycle after the rising edge 
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("Linefeed.vcd");

    //by default the output for a linefeed is low
    REQUIRE(tb->dut->o_linefeed_rising == 0); 

    //data only latches if the input wire is high
    tb->dut->i_data_latch = 0x00; 
    tb->dut->i_data = 0x7F; 
    tb->tick(); 
    REQUIRE(tb->dut->linefeed_detector__DOT__data_sr[0] == 0x00);
    
    tb->dut->i_data_latch = 0x01; 
    tb->tick(); 
    REQUIRE(tb->dut->linefeed_detector__DOT__data_sr[0] == 0x7F);

    //If we put random data in it that's not ``/r/n`` it does not go high
    tb->dut->i_data_latch = 0x01; 
    auto outputLow = true;
    for( int j = 0; j<=255;j++)
    {
        tb->dut->i_data = j;
        tb->tick();
        outputLow &= !tb->dut->o_linefeed_rising;
    }
    REQUIRE(outputLow);

    //putting a linefeed into the data register will put the logic level high on a one clock delay
    const std::string endOfLine = "\r\n";
    tb->dut->i_data_latch = 0x01; // make sure data latch is high
    for(const char &c:endOfLine){
        tb->dut->i_data = c; // put the char onto the data bus
        tb->tick();
    }
    REQUIRE_FALSE(tb->dut->o_linefeed_rising); //the flag should be delayed one tick
    tb->tick();
    REQUIRE(tb->dut->o_linefeed_rising);
    tb->tick();
    REQUIRE_FALSE(tb->dut->o_linefeed_rising); //flag should only stay on for one tick

    //if we reverse the order it should not work;
    delete tb;
    const std::string fakeEOL = "\n\r";
    tb->dut->i_data_latch = 0x01; // make sure data latch is high
    for(const char &c:fakeEOL){
        tb->dut->i_data = c; // put the char onto the data bus
        tb->tick();
    }
    tb->tick();
    REQUIRE_FALSE(tb->dut->o_linefeed_rising); //the flag should not have been set
}
