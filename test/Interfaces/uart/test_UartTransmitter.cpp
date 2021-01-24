//
// Created by rfraz on 1/24/2021.
//

#include "interfaces/uart/UartTransmitter.hpp"
#include <catch2/catch.hpp>
#include <memory>
#include <string>

static void transmit_byte(tb::interface::UartTransmitter& transmitter, int& tx_wire, char to_transmit){
    int baud = transmitter.get_baud_rate();
    //drive the clock low for an entire baud cycle
    tx_wire = 0;
    for(int i=0;i<baud; i++){
        transmitter.post_tick();
    }

    // transmit the data bits
    for(int bit=0;bit<8;bit++){
        tx_wire = (to_transmit >> bit) & 0x01; //mask for the appropriate bit to transmit
        // tick for a baud cycle
        for(int i=0;i<baud; i++){
            transmitter.post_tick();
        }
    }

    //transmit the stop bit
    tx_wire = 1;
    for(int i=0;i<baud; i++){
        transmitter.post_tick();
    }
}

TEST_CASE("Writing a Byte to the Uart Transmitter","[interface][uart]"){
    /*
     * write a byte to the UART and verify it gets enqueued
     */
    int tx_wire=1; //a mock tx wire for a DUT
    int baud = 64;
    auto transmitter = std::make_unique<tb::interface::UartTransmitter>(tx_wire, baud); //create a transmitter
    REQUIRE(!transmitter->byte_available());
    char to_transmit = 'A';

    transmit_byte(*transmitter, tx_wire, to_transmit);
    REQUIRE(transmitter->byte_available());
    REQUIRE(transmitter->pop_byte() == to_transmit);
}

TEST_CASE("Writing Multiple bytes to the Uart Transmitter","[interface][uart]"){
    /*
     * Write multiple bytes without de-queueing and ensure they're popped in the correct order
     */
    int tx_wire=1; //a mock tx wire for a DUT
    int baud = 64;
    auto transmitter = std::make_unique<tb::interface::UartTransmitter>(tx_wire, baud); //create a transmitter
    REQUIRE(!transmitter->byte_available());
    std::string tx_message = "Hello World";
    for(char i: tx_message){
        transmit_byte(*transmitter, tx_wire, i);
    }

    for(char i: tx_message){
        REQUIRE(transmitter->pop_byte() == i);
    }
}