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

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}


//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("UART_SINGLETX.vcd");
    tb->dut->i_data = 0xA5; //put data on the bus
    
    //start a write cycle
    tb->dut->i_write = 1;
    tb->tick();

    //loop until write is done
    while(tb->dut->o_busy==1){
        tb->tick();
    }
}