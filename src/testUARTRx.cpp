#define MODTYPE Vuart_rx

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "uart_rx/Vuart_rx.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 8
#define ST_IDLE 10
//State machine registers and paramters

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}


void receiveByte(SyncTB<MODTYPE>* testbench, int dataByte){
    auto dataPacket = (dataByte | 0x100)<<1;
    for(int k=0;k<10;k++)
    {
        testbench->dut->i_rx_w=((dataPacket>>k) & 0x01); // update the bit on the data register
        for(int j=0;j<TICKS_PER_CYCLE;j++)
        {
            testbench->tick();
        }
    }
}

TEST_CASE("Single Byte Transmission","[uart-rx][uart]"){
    /*
    Makes a GTKWave plot of a uart Receive frame and verifies the correct value was latched
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("UART_SINGLERX.vcd");
    tb->tick(); // Tick the clock once
    
    //Transmit a byte and make sure the right value is received
    auto transmissionByte = 0xA5;
    receiveByte(tb, transmissionByte);
    REQUIRE(tb->dut->o_data_w==transmissionByte);
}

TEST_CASE("Data Ready Wire Assertion","[uart-rx][uart]"){
    /*
    The data ready wire should go high for one clock cycle after 9.5*CYCLES_PER_BIT clock cycles
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    REQUIRE(tb->dut->o_data_ready_w==0x00); //Data ready should initialize to zero
    tb->dut->i_rx_w = 0x00;
    int cyclesPerDataFrame = 9.5*TICKS_PER_CYCLE;
    for(int j = 0; j<cyclesPerDataFrame;j++)
    {
        tb->tick();
    }
    REQUIRE(tb->dut->uart_rx__DOT__system_state_r==7); //should be in the last clock cycle of b7 transmit
    tb->tick();
    REQUIRE(tb->dut->uart_rx__DOT__system_state_r==9); //should have now entered the latch data state
    REQUIRE(tb->dut->o_data_ready_w == 0); // Data ready should not assert until the next cycle
    tb->tick();
    REQUIRE(tb->dut->o_data_ready_w == 1);
}

TEST_CASE("busy wire stays high through entire dataframe","[uart-rx][uart]"){
    /*
    The busy wire should be combinatorial output and assert itself whenever the state is not idle 
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    REQUIRE(tb->dut->o_busy == 0x00); // By default we're not busy
    //Kick off a data transfer
    tb->dut->i_rx_w=0x00;
    tb->tick();
    bool busyFlagValid = true;

    //iterate over 1000 clock cycles with the data ling toggling every cycle
    for(int j=0;j<1000;j++)
    {
        tb->tick();
        busyFlagValid &= !(tb->dut->o_busy && (tb->dut->uart_rx__DOT__system_state_r==0x0A));
        busyFlagValid &= !(!tb->dut->o_busy && (tb->dut->uart_rx__DOT__system_state_r!=0x0A));
        tb->dut->i_rx_w ^= 1; // XOR the input wire to toggle 
    }
    REQUIRE(busyFlagValid);
}

TEST_CASE("There's no counter issues for multiple sequential bytes","[uart-rx][uart]"){
    /*
    If recieving multiple bytes in a row the counter should not lag and eventually cause incorrect state transitions
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    auto dataTransferWorking=true;
    for(int j= 0;j<0x100;j++){
        receiveByte(tb, j);
        dataTransferWorking &= (tb->dut->o_data_w == j);
    }
    REQUIRE(dataTransferWorking);
}

TEST_CASE("Initial States are set properly","[uart-rx][uart]"){
    /*
    All values should be initialized properly
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    REQUIRE(tb->dut->o_data_ready_w==0);
    REQUIRE(tb->dut->o_data_w == 0x00);
    REQUIRE(tb->dut->uart_rx__DOT__system_state_r == ST_IDLE);
    REQUIRE(tb->dut->uart_rx__DOT__baud_counter_r == (int)(1.5*TICKS_PER_CYCLE-1));
}
