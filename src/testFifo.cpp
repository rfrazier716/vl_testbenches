#define MODTYPE Vfifo

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

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


TEST_CASE("Single Byte Transmission","[fifo]"){
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

TEST_CASE("Reading and writing an empty FIFO","[fifo]"){
    /*
    When the system wants to simultaneously read and write from an empty FIFO, the value should be stored and
    and both the read and write head advance
    */

    auto* tb = new SyncTB<MODTYPE>(50000000, false);
    auto write_value = 0x7F;
    tb->dut->i_data_w = write_value;
    tb->dut->i_write_w = 1;
    tb->dut->i_read_w = 1;
    tb->tick();
    REQUIRE(tb->dut->o_data_w == write_value);
}

TEST_CASE("Tracking bytes stored in FIFO","[fifo]"){
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

/* Cases to Test
- reading & writing when FIFO empty -- Should this work? would act like a pass through
- reading & writing when FIFO full -- This should work, 
- Crossing the memory boundary (overflow on FIFO_DEPTH)
- Reading while empty ( do nothing )
- Writing while full ( do nothing )
- general writing and advancing write head
- general reading and advancing read head
*/

//TODO: Synthesize code and make sure it synthesizes to block ram