// Necessary Includes for Verilated
#define MODTYPE Vhello_uart
#define CYCLES_PER_BIT 32
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "Vhello_uart.h"
#include <verilated.h>

//Include the TCPTest Bench
#include "inc/tcp/TCPUart.hpp"

#include <iostream>
#include <string>
#include <memory>


template <class T>
void log(const T item,const std::string& prefix = "",const std::string& suffix = ""){
    std::cout << prefix << item << suffix << std::endl; 
}

int main(){
    auto uart = std::make_unique<TCPUart<MODTYPE>>(8080, 8);
    uart->addVCDTrace("Uart_Echo.vcd");
    log("Hello World");
    for(int j=0; j<10000;j++){
        uart->tick();
    }
}