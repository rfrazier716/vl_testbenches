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
#include <catch2/catch.hpp>

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 3
//State machine registers and paramters
#define ST_TRANSMIT_B0 0x00
#define ST_TRANSMIT_B1 0x01
#define ST_TRANSMIT_B2 0x02
#define ST_TRANSMIT_B3 0x03
#define ST_TRANSMIT_B4 0x04
#define ST_TRANSMIT_B5 0x05
#define ST_TRANSMIT_B6 0x06
#define ST_TRANSMIT_B7 0x07
#define ST_DEADZONE  0x08
#define ST_PARITY  0x09
#define ST_STOP 0x0A
#define ST_IDLE 0x0B
#define ST_INIT 0x0C

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
TEST_CASE("UartTX Single Byte Transmission","[uart-tx][uart]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("UART_SINGLETX.vcd");
    tb->tick(); // Tick the clock once
    tb->dut->i_data = 0xA5; //put data on the bus
    writeUart(tb);
    tb->dut->i_data = 0x5A;
    writeUart(tb);
    //
}

TEST_CASE("Busy Flag Functional","[uart-tx][uart]"){
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

TEST_CASE("Clock Frequency Accurate","[uart-tx][uart]"){
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

TEST_CASE("Part initializes to idle","[uart-tx][uart]"){
    /*
    Makes sure that the default state is idle and it doesn't accidently transmit data
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    REQUIRE(tb->dut->uart_tx__DOT__system_state_r == ST_IDLE);
    REQUIRE(tb->dut->o_tx_w == 1); //output should default to high
    REQUIRE(tb->dut->o_busy==0); //busy should initialize to low
    auto stillIdle = true;
    for(int j = 0; j < 100; j++)
    {
        tb->tick();
        stillIdle &= (tb->dut->uart_tx__DOT__system_state_r == ST_IDLE);
    }
    REQUIRE(stillIdle);
}

TEST_CASE("Only Initialize write when idle","[uart-tx][uart]"){
    /*
    Verifies that if a write request comes in when the part is already sampling it doesn't restart the state machine
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->dut->i_data = 0xFF; //write all high 
    tb->dut->i_write = 0x01; //tell the device to write on the next clock
    tb->tick(); // tick to latch data
    
    //cycle the clock until you're out of the init cycle
    while(tb->dut->uart_tx__DOT__system_state_r == ST_INIT){
        tb->tick();
    }

    //now complete the rest of the write cycle making sure not to 
    auto transmitCycleUninterrupted = true;
    while(tb->dut->o_busy == 1){
        tb->dut->i_write = 0x01;
        tb->tick();
        transmitCycleUninterrupted &= (tb->dut->uart_tx__DOT__system_state_r != ST_INIT); // test fails if we ever re-enter init
    }
    REQUIRE(transmitCycleUninterrupted);

}

//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output