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


TEST_CASE("Single Byte Transmission","[uart-tx]"){
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