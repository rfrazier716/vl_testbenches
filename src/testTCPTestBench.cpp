// Standard IO Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>
#include <memory>

// Necessary Includes for Verilated
#define MODTYPE Vwb_uart
#define TCP_PORT 8080
#define TICKS_PER_CYCLE 108

//If not search key was defined set it to a carriage return
#include "Vwb_uart.h"
#include <verilated.h>

// Test Bench Class Definition and Implementation
#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/tcp/TCPServer.hpp"

// Catch Library
#include <catch2/catch.hpp>
#include <boost/range/irange.hpp>


TEST_CASE("Connecting to a TCP TestBench","[test-bench]"){
    auto server = std::make_unique<TCPServer>(); //make a server on port 8080
    server->start(); // start the server
    for (auto i: boost::irange(10000)){
        auto message = std::to_string(i)+"\r\n";
        server->write_to_connection(message);
    }
    server->stop();
}