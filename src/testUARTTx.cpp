#define MODTYPE Vuart_tx

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "uart_tx/Vuart_tx.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 3

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}

void writeUart(SyncTB<MODTYPE>* testBench){
    //start a write cycle
    testBench->dut->i_write = 1;
    testBench->tick();
    testBench->dut->i_write = 0;
    while(testBench->dut->o_busy==1){
        testBench->tick();
    }
}

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("UART_SINGLETX.vcd");
    tb->tick(); // Tick the clock once
    tb->dut->i_data = 0xA5; //put data on the bus
    writeUart(tb);
    
    // Tick a few times to put a gap
    for(int j;j<5;j++){
        tb->tick();
    }

    tb->dut->i_data = 0x5A;
    writeUart(tb);
    //
}

TEST_CASE("Busy Flag Functional","[uart-tx]"){
    /*
    Tests that the busy flag is initially idle, goes high when the state machine is active,
    and goes low once the state machine idles again
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    REQUIRE(tb->dut->o_busy == 0); //initially the busy flag should be low
    
    //put data on the input bus and set the write signal high
    tb->dut->i_data = 0xAA; 
    tb->dut->i_write = 1;
    tb->tick();

    REQUIRE(tb->dut->o_busy == 1); //There should now be a busy flag
    auto expectedBusyTicks = TICKS_PER_CYCLE*(8+2); // We have 8 data bits, one init, one stop, and N ticks per cycle
    auto busyStaysHigh = true; 
    for(int j = 0;j<expectedBusyTicks-1;j++){
        tb->tick();
        if(tb->dut->o_busy != 1)
        {
            busyStaysHigh = false;
        }
    }
    REQUIRE(busyStaysHigh); // Busy signal should have stayed high that entire time
    tb->tick(); //after the final tick it should now be low
    REQUIRE_FALSE(tb->dut->o_busy);
}

TEST_CASE("Clock Frequency Accurate","[uart-tx]"){
    /*
    Verifies that the state machine transitions after the counter overflows
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    int tickCounter = 0; //initialize a counter to see how long the part stays at one output
    int pastOutput; // The previous state of the output
    
    auto currentOutput=1;
    tb->dut->i_data = 0x55; //Put a signal on the data bus that toggles every other bit
    tb->dut->i_write = 1;
    tb->tick(); //latch the data
    tb->tick(); // Tick again so the state machine updates
    pastOutput = tb->dut->o_tx_w;

    auto ticksPerBitAccurate = true;
    while(tb->dut->o_busy == 1)
    {  
        currentOutput = tb->dut->o_tx_w; //update the current output value
        if(currentOutput != pastOutput)
        {
            ticksPerBitAccurate &= (tickCounter == 3);
            tickCounter = 0; // Reset the counter
            pastOutput = currentOutput;
        }
        tb->tick(); //Tick the clock again
        tickCounter++;
    }
    REQUIRE(ticksPerBitAccurate);
}

TEST_CASE("Part initializes to idle","[uart-tx]"){
    /*
    Makes sure that the default state is idle and it doesn't accidently transmit data
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    auto idleState = 0b1011;
    REQUIRE(tb->dut->uart_tx__DOT__system_state_r == idleState);
    REQUIRE(tb->dut->o_tx_w == 1); //output should default to high
    REQUIRE(tb->dut->o_busy==0); //busy should initialize to low
    for(int j = 0; j < 100; j++)
    {
        tb->tick();
    }
    REQUIRE(tb->dut->uart_tx__DOT__system_state_r == idleState);
}

TEST_CASE("Only Initialize write when idle","[uart-tx]"){
    /*
    Verifies that if a write request comes in when the part is already sampling it doesn't restart the state machine
    */
    //TODO: Write this case

}

//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output