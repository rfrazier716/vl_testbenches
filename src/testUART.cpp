#define MODTYPE Vwb_uart

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "Vwb_uart.h"
#include <verilated.h>

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"
#include "inc/UartTestBench.hpp"

// Catch Library
#include <catch2/catch.hpp>

#include <boost/range/irange.hpp>

// Additional parameter definition, this is also set in the verilator .sh file
#define TICKS_PER_CYCLE 8
#define FIFO_DEPTH 8

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

void receiveVUartChar(UartTestBench<MODTYPE>* tb, char rxByte){
    tb->vUart->writeRxBuffer(rxByte); //write to the rx buffer
    tb->tick(); //tick once to get out of the idle state
    for(int i: boost::irange(TICKS_PER_CYCLE*10)) tb->tick(); //tick through the receive clocks
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

TEST_CASE("Long Message Transmission","[uart-top][uart]"){
    /*
    make sure that we can transmit a long message without issues with the buffer or losing a byte
    */

    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_multi_tx.vcd");
    auto messageLength = 0x80;
    for(int i: boost::irange(messageLength)){
        wbSlaveWriteRequest(tb,0x12,i); // write into the tx Buffer
    }
    // there are 2 ticks per write request, and a data packet takes 10*TICKS_PER_CYCLE+1 to transmit
    // the first write request wasn't in parallel with a transmit so it does not count
    for(int i=0;i<(messageLength*(10*TICKS_PER_CYCLE +1)-(messageLength-1)*2);i++){
        tb->tick();
    }    
    //require that we're done transmmitting
    REQUIRE(!tb->dut->led_tx_busy);
    
    //not iterate over the data on the tx buffer and make sure it's in the correct order
    auto bytesTransmittedSuccessfully = true;
    for(int i: boost::irange(messageLength)){
        bytesTransmittedSuccessfully &= (tb->vUart->txBuffer[i]==i);
    }
    REQUIRE(bytesTransmittedSuccessfully);
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
    auto bytesToReceive = 0x10;
    for(int i: boost::irange(bytesToReceive)){
        tb->vUart->writeRxBuffer(i);
        tb->tick(); //tick once to get out of the idle state
        for(int i: boost::irange(TICKS_PER_CYCLE*10)) tb->tick(); //tick through the 10 bits transmitted in a frame
    }
    REQUIRE(!tb->dut->led_rx_busy); //at the end of the transmission the RX state should not be busy

    //initiate a wishbone read of register 0x01 which should be be the count of bytes on the fifo
    auto rxByte = wbSlaveReadRequest(tb, 0x0001);
    tb->tick();
    REQUIRE(rxByte == bytesToReceive);

    //Now read back all those bytes
    for(int i: boost::irange(bytesToReceive)){
        tb->vUart->writeRxBuffer(i);
        tb->tick(); //tick once to get out of the idle state
        for(int i: boost::irange(TICKS_PER_CYCLE*10)) tb->tick(); //tick through the 10 bits transmitted in a frame
    }

}

TEST_CASE("linefeed interrupt functional","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_newline.vcd");
    tb->tick();

    std::string rxMessage = "Hello world, this is quite the long message is it not?\r";
    for(auto &c: rxMessage){
       receiveVUartChar(tb, c);
    }
    REQUIRE(tb->dut->rx_linefeed_available); //the linefeed availabe flag should be set
    REQUIRE(tb->dut->rx_fifo_byte_available); // the byte available flag should be set

    //now if we write another bit that flag should be deasserted
    receiveVUartChar(tb, 'x');
    REQUIRE(!tb->dut->rx_linefeed_available); //the lf flag shoudl deassert
    REQUIRE(tb->dut->rx_fifo_byte_available); // the byte available flag should still be set
}

TEST_CASE("FIFO Full Interrupt functional","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    tb->addVCDTrace("WB_UART_newline.vcd");
    tb->tick();

    auto fifo_bytes = (0x01 << FIFO_DEPTH)-1;
    for(auto i: boost::irange(fifo_bytes)){
        receiveVUartChar(tb,i);
    }
    //check the appropriate flags are set
    REQUIRE(!tb->dut->rx_linefeed_available); //no linefeed should be found
    REQUIRE(tb->dut->rx_fifo_full); //fifo should be full
    REQUIRE(tb->dut->rx_fifo_byte_available); //bytes are available
    
}

TEST_CASE("Bytes on Rx Fifo Count","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench

    //fill the Rx fifo
    auto fifoFillWorking = true;
    auto bytesToReceive = (0x01 << FIFO_DEPTH)-1;
    for(int i: boost::irange(bytesToReceive)){
        receiveVUartChar(tb,i);
        fifoFillWorking &= (wbSlaveReadRequest(tb,0x01) == (i+1));
    }
    REQUIRE(fifoFillWorking);

}

TEST_CASE("Bytes on Tx Fifo Count","[uart-top][uart]"){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench
    //it takes 3 clock cycles to write a byte
    //8 clock cycles to transmit a single character
    //so including start and stop bit we have 80 clock cycles before the next byte gets popped
    //I can write 20 bytes to the fifo in that time easily, and the fifo depth should be n-1 where 
    // n is bytes written

    //fill the Tx fifo
    auto fifoFillWorking = true;
    auto bytesToWrite = 20; //write 20 bytes out
    int bytesOnTxFifo;
    wbSlaveWriteRequest(tb, 0x12, 0x00); //write data to keep the tx busy while writing more
    tb->tick(); //tick so the data is pulled from the fifo
    for(int i: boost::irange(bytesToWrite)){
        wbSlaveWriteRequest(tb,0x12,i);
        bytesOnTxFifo = wbSlaveReadRequest(tb,0x02);
        fifoFillWorking &= (bytesOnTxFifo == (i+1));
    }
    REQUIRE(fifoFillWorking);

}