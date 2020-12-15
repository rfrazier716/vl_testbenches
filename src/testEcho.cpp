#define MODTYPE Vuart_echo

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory> //smart pointers

// Necessary Includes for Verilated
#include "Vuart_echo.h"
#include <verilated.h>

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/UartTestBench.hpp"
typedef decltype(UartTestBench<MODTYPE>()) TEST_BENCH;
typedef decltype(std::unique_ptr<TEST_BENCH>()) TEST_BENCH_P;

// Catch Library
#include <catch2/catch.hpp>

#include <boost/range/irange.hpp>

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 12

TEST_CASE("Local Echo works for single bit rx","[examples][uart]"){
    /*
    Put a character on the Rx Buffer and make sure it gets retransmitted on the Tx Buffer 
    */
    TEST_BENCH_P tb = std::make_unique<TEST_BENCH>(TICKS_PER_CYCLE);
    tb->addVCDTrace("UartEcho.vcd"); //add a trace file

    tb->vUart->writeRxBuffer('a');
    //tick twice to enter a data latch frame and then loop until the rx wire isn't busy
    tb->tick();
    tb->tick();
    while(tb->dut->led_rx_busy){
        tb->tick();
    }
    REQUIRE(tb->dut->uart_echo__DOT__system_state_r == 0x03); //we should still be idle
    //tick once more which is should force the top-level module to detect the interrupt
    tb->tick();
    tb->tick();
    //now check that we're in the read request state
    REQUIRE(tb->dut->uart_echo__DOT__system_state_r == 0x00);

    //loop until we're back to idling
    while(tb->dut->uart_echo__DOT__system_state_r!=0x03){
        tb->tick();
    }
    //after two more ticks we should be transmitting
    tb->tick();

    REQUIRE(tb->dut->led_tx_busy==0x01);
    //loop over the transmission
    while(tb->dut->led_tx_busy){
        tb->tick();
    }

    //check that we got the same character off of vUart Tx Buffer
    REQUIRE(tb->vUart->getLastTxByte() == 'a');
    
    //make sure we don't transmit any more than that one byte
    auto stateIdle = true;
    for(int j= 0;j<1000;j++){
        tb->tick();
        stateIdle &= !tb->dut->led_tx_busy;
    }
    REQUIRE(stateIdle);
}