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
#include "inc/UartTestBench.hpp"

// Catch Library
#include <catch2/catch.hpp>

#include <boost/range/irange.hpp>

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 8

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}

void wbSlaveWriteRequest(UartTestBench<MODTYPE>* tb, int addr, int value){
    //Clock cycle 0: assert strobe, write enable, and put address and value on bus
    tb->dut->wb_addr_in = addr;
    tb->dut->wb_data_in = value;
    tb->dut->wb_write_enable_in = 1;
    tb->dut->wb_strobe_in = 1; 
    // this tick is really clock edge 1 becaue those inputs were applied before the clock
    tb->tick(); //tick the clock
    
    //Clock Edge 2
    tb->dut->wb_strobe_in = 0;
    tb->dut->wb_write_enable_in = 0;
    tb->tick();
}

int wbSlaveReadRequest(UartTestBench<MODTYPE>* tb, int addr){
    //Clock cycle 0: assert strobe, write enable, and put address and value on bus
    tb->dut->wb_addr_in = addr;
    tb->dut->wb_write_enable_in = 0;
    tb->dut->wb_strobe_in = 1; 
    // this tick is really clock edge 1 becaue those inputs were applied before the clock
    tb->tick(); //tick the clock
    
    //Clock Edge 2
    tb->dut->wb_strobe_in = 0;
    // this data disappears from the buss after this clock edge so we need to "latch" it early
    auto rxByte = tb->dut->wb_data_out; //latch the data
    tb->tick(); //tick the clock to finish the second clock cycle
    return rxByte;
    
}

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-top][uart]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_tx.vcd");
    wbSlaveWriteRequest(tb,0x12,0x7F); // this shoudl push a bit onto the buffer
    tb->tick();     

    //We should have ticked enought cycles to perform that write that the tx block runs
    while(tb->dut->led_tx_busy){
        tb->tick();
    }
    
    //make sure that the UART was latched properly to the virtualUart
    REQUIRE(tb->vUart->getLastTxByte() == 0x7F);
}

TEST_CASE("Receiving a Byte","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_rx.vcd");
    tb->tick();
    tb->vUart->writeRxBuffer(0x01); // put a byte onto the rxBuffer
    // in two ticks the vUart should have popped the byte off the buffer and asserted a low
    // on the RX wire
    tb->tick();
    REQUIRE_FALSE(tb->dut->led_rx_busy); // should not yet be busy
    tb->tick();
    REQUIRE(tb->dut->led_rx_busy); //now the data should be on rx line

    //tick through the data capture until a bytes available
    while(!tb->dut->rx_fifo_byte_available) tb->tick();

    //initiate a wishbone read of register 0x11 which should be the most recent fifo bit
    auto rxByte = wbSlaveReadRequest(tb, 0x0001);
    tb->tick();
    REQUIRE(rxByte == 0x01);
}

TEST_CASE("Receiving multiple bytes","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_rx.vcd");
    tb->tick();

    //put a series of bytes on the buffer
    for(int i: boost::irange(0x04)){
        tb->vUart->writeRxBuffer(i);
        tb->tick();
        tb->tick();
        while(tb->dut->led_rx_busy) tb->tick();
        }

    //initiate a wishbone read of register 0x11 which should be the most recent fifo bit
    auto rxByte = wbSlaveReadRequest(tb, 0x0001);
    tb->tick();
    REQUIRE(rxByte == 0x10);
}


//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output