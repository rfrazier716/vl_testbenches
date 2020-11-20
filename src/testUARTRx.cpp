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
//State machine registers and paramters

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("UART_SINGLERX.vcd");
    tb->tick(); // Tick the clock once
    tb->dut->i_rx_w = 0; //put data on the bus
    auto data_message = (0xA5+(1<<8))<<1; // assign a message and put the stop bit at the end, and start bit at beginning
    // iterate over the incoming message
    for(int k=0;k<10;k++)
    {
        tb->dut->i_rx_w=((data_message>>k) & 0x01); // update the bit on the data register
        for(int j=0;j<TICKS_PER_CYCLE;j++)
        {
            tb->tick();
        }
    }
    // add extra ticks
    for(int j=0;j<TICKS_PER_CYCLE;j++)
    {
        tb->tick();
    }
}