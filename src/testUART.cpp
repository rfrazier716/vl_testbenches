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
#include "inc/catch.hpp"

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
    tb->tick();
    return tb->dut->wb_data_out; // return the data out
}

void receiveByte(UartTestBench<MODTYPE>* testbench, int dataByte){
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

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->dut->i_rx_w = 1; // The reciever wire should be nominally high
    tb->dut->eval();
    tb->addVCDTrace("WB_UART.vcd");
    tb->tick();
    tb->tick();
    tb->attachServer();
    // put in a few write requests
    std::string message = "Hello World\r\n";
    for (char const &c: message)
    {
        wbSlaveWriteRequest(tb,0x12, c);
    }
    while(!tb->dut->wb_uart__DOT__tx_fifo_empty) tb->tick(); //Tick until the fifo is empty
    while(tb->dut->wb_uart__DOT__uart_tx_busy_w) tb->tick(); //Tick until the last byte finishes transmitting

}

//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output