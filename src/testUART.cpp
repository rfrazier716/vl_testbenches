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
#define TICKS_PER_CYCLE 8

template<class T>
void logSignal(T signal)
{
    printf("0x%02x\n",signal);
}

void wbSlaveWriteRequest(SyncTB<MODTYPE>* tb, int addr, int value){
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

int wbSlaveReadRequest(SyncTB<MODTYPE>* tb, int addr){
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

//Cases to Test
//Program counter reset
TEST_CASE("Single Byte Transmission","[uart-tx]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->dut->i_rx_w = 1; // The reciever wire should be nominally high
    tb->dut->eval();
    tb->addVCDTrace("WB_UART.vcd");
    
    // put in a few write requests
    wbSlaveWriteRequest(tb, 0x12, 0xA5);
    wbSlaveWriteRequest(tb, 0x12, 0x5A);
    wbSlaveWriteRequest(tb, 0x12, 0x7F);

    // add data to the FIFO
    receiveByte(tb,0x5A);
    receiveByte(tb,0xA5);

    auto bytesOnReceiver = wbSlaveReadRequest(tb,0x01);
    REQUIRE(bytesOnReceiver == 2); // There should be two bytes on the receiver

    // pull a byte off the fifo
    auto fifoData = wbSlaveReadRequest(tb,0x11);
    REQUIRE(fifoData == 0x5A);



}

//TODO: Write case to make sure states progress in order
//TODO: write case to make sure the proper data is output