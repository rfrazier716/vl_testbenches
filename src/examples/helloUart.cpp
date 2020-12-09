#define MODTYPE Vhello_uart

// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Necessary Includes for Verilated
#include "hello_uart/Vhello_uart.h"
#include "verilated.h"

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#define TICKS_PER_CYCLE 8
#define COUNTER_INITIAL_VALUE 100
#include "inc/UartTestBench.hpp"



int main(int argc, char* argv[]){
    auto* tb = new UartTestBench<MODTYPE>(TICKS_PER_CYCLE); // make a new module test bench with a 108 baud counter
    //tb->addVCDTrace("hello_uart.vcd");
    for(int i=0;i<10*COUNTER_INITIAL_VALUE;i++){
        tb->tick(); //tick the test bench
        // if the TX buffer isn't empty pop a bit and print it to the console
        if(!tb->vUart->txBuffer.empty()){
            std::cout << "REC: " << tb->vUart->txBuffer[0] <<std::endl;
            tb->vUart->txBuffer.pop_back(); //pop the bit off the fifo
        }
    }
}