#define MODTYPE Vwb_uart

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "wb_uart/Vwb_uart.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"

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

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("WB_UART.vcd");
    tb->dut->wb_uart__DOT__tx_fifo_write_w = 1;
    for(int j=0;j<3;j++)
    {
        tb->tick();
    }
    tb->dut->wb_uart__DOT__tx_fifo_write_w = 0;
    for(int j=0;j<=300;j++)
    {
        tb->tick();
    }
    //
}

//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output