#define MODTYPE Vfifo

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//Boost includes
#include <boost/range/irange.hpp> //integer range

// Necessary Includes for Verilated
#include "fifo/Vfifo.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include "inc/catch.hpp"

//Define FIFO Parameter
#define FIFO_DEPTH 3
const int  g_fifoBits=(0x01<<FIFO_DEPTH);


const int fifoMemory = (1<<FIFO_DEPTH)-1;


void writeFIFO(SyncTB<MODTYPE>* tb, int writeData)
{
    tb->dut->i_data_w = writeData;
    tb->dut->i_write_w = 1;
    tb->tick();
    tb->dut->i_write_w = 0;
}

int readFIFO(SyncTB<MODTYPE>* tb)
{
    auto readData = tb->dut->o_data_w;
    tb->dut->i_read_w = 1;
    tb->tick();
    tb->dut->i_read_w = 0;
}


TEST_CASE("Single Byte Transmission","[fifo][uart]"){
    /*
    This is really just here to make a gtkwave plot in the build directory that can be referenced for debug
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false); // make a new module test bench
    tb->addVCDTrace("FIFO_SingleWrite.vcd");
    tb->tick(); // Tick the clock once
    tb->dut->i_data_w = 0x7F;
    tb->dut->i_write_w = 1;
    tb->tick();
    tb->dut->i_read_w = 1;
    for(int j=0;j<127;j++)
    {
        tb->dut->i_data_w = j;
        tb->tick();
    }
}


TEST_CASE("Tracking bytes stored in FIFO","[fifo][uart]"){
    /*
    When the system wants to simultaneously read and write from an empty FIFO, the value should be directly placed on o_data
    instead of stored
    */
    auto* tb = new SyncTB<MODTYPE>(50000000, false);
    tb->addVCDTrace("Filling_FIFO.vcd");

    //fill the FIFO and make sure the bytes in the fifo track
    auto bytesInFifoTracking = true;
    for(int j=0; j<fifoMemory; j++)
    {
        writeFIFO(tb, j);
        bytesInFifoTracking &= (tb->dut->o_fill_bytes_w == j+1);
    }
    REQUIRE(bytesInFifoTracking);

    //now empty the fifo half way, making sure that it still tracks
    bytesInFifoTracking = true;
    for(int j=0; j<(fifoMemory>>1); j++)
    {
        readFIFO(tb); // read the FIFO
        bytesInFifoTracking &= (tb->dut->o_fill_bytes_w == fifoMemory-(j+1));
    }
    REQUIRE(bytesInFifoTracking);

    //and now refil the FIFO so there's a rollover and make sure the counter follows the rollover
    bytesInFifoTracking = true;
    for(int j=tb->dut->o_fill_bytes_w; j<fifoMemory; j++)
    {
        writeFIFO(tb,j); // read the FIFO
        bytesInFifoTracking &= (tb->dut->o_fill_bytes_w == j+1);
    }
    REQUIRE(bytesInFifoTracking);
}

TEST_CASE("Filling and Writing A FULL FIFO","[fifo][uart]"){
    /*
    When writing to a full fifo it should act like a shift register, one bit is put in and one is pulled out 
    Need to remember, when you read data from the fifo and advance the read head, it is the signal BEFORE the tick that ends up on your read head
    */

    //Make a new fifo test bench
    auto* tb = new SyncTB<MODTYPE>(50000000, false);
    tb->addVCDTrace("Filling_FIFO.vcd");

    //Fill the fifo, there should be 2^FIFO_DEPTH-1 bytes available
    //iterate from 1 -> 2^FIFO_DEPTH, and double check that as you put a byte in, the counter updates as well
    for(auto j: boost::irange(1, g_fifoBits)){
        tb->dut->i_data_w = j; //put the byte into the fifo
        tb->dut->i_write_w = 1; //set write high;
        tb->tick();
        REQUIRE(tb->dut->fifo__DOT__write_head_r == j);
        REQUIRE(tb->dut->o_fill_bytes_w == j); //REQUIRE that the number of bytes in the fifo is increased by 1
    }
    //After writing that last byte the FIFO should be full
    writeFIFO(tb,0xFF); // write 0xFF to the FIFO
    REQUIRE(tb->dut->o_full_w);

    //If we try to write another byte:
    // - the write head should not advance
    // - the data in furthest buffer position should stay unchanged
    // - Buffer stays full
    writeFIFO(tb,0xFF); // write 0xFF to the FIFO
    REQUIRE(tb->dut->fifo__DOT__fifo_buffer_r[g_fifoBits-2] == 7);
    REQUIRE(tb->dut->o_full_w);

    //If we try and write while reading a byte:
    // - the write head will advance
    // - the read head will advance
    // - the buffer is still full
    // - new write head is 0x00
    tb->dut->i_data_w = 0x7F;
    tb->dut->i_write_w = 1;
    tb->dut->i_read_w = 1;
    tb->tick();
    REQUIRE(tb->dut->o_full_w);
    REQUIRE(tb->dut->fifo__DOT__write_head_r == 0x00);
    REQUIRE(tb->dut->fifo__DOT__read_head_r == 0x01);
}

TEST_CASE("Reading an Empty FIFO","[fifo][uart]"){
    /*
    When trying to read from an empty fifo the read head does not advance and the data on the bus is garbage,
    */

    //make a new test bench
    auto* tb = new SyncTB<MODTYPE>(50000000, false);

    //Set the read wire high and submit a read request
    tb->dut->i_read_w = 1;
    readFIFO(tb);

    //FIFO should still be empty and read head not advanced
    CHECK(tb->dut->o_empty_w);
    REQUIRE(tb->dut->fifo__DOT__read_head_r==0x00);
}

TEST_CASE("FIFO fill output on Rollover","[fifo][uart]"){
    /* 
    Fifo Fill Factor should stay accurate even when rolling over the buffer boundary
    */

   //Make a new Testbench
   auto* tb = new SyncTB<MODTYPE>(50000000, false);
   
   //file it with 5 bytes
   tb->dut->i_write_w = 1;
   auto bytesToWrite=5;
   for(auto i: boost::irange(bytesToWrite)){
       tb->tick();
   }
   REQUIRE(tb->dut->o_fill_bytes_w == bytesToWrite);

   //now just loop through 50+ times and make sure that it always stays at the right value
   auto fillOutputAccurate = true;
   for(auto i: boost::irange(100)){
       fillOutputAccurate &= (tb->dut->o_fill_bytes_w == bytesToWrite);
   }
   REQUIRE(fillOutputAccurate);
}
