//
// Created by rfraz on 1/31/2021.
//

#include "interfaces/uart/UartReceiver.hpp"

#include <catch2/catch.hpp>
#include <memory>
#include <iostream>

TEST_CASE("Uart Receiver Creation and getters","[interface][uart]"){
    const int baud = 64;

    int rx_wire = 1; //a mock tx wire for a DUT
    auto receiver = std::make_unique<tb::interface::UartReceiver>(rx_wire, baud); //create a transmitter

    REQUIRE(receiver->get_baud_rate() == baud);
    REQUIRE(receiver->get_rx_wire() == rx_wire);
    rx_wire = 0;

    REQUIRE(receiver->get_rx_wire() == rx_wire);
    REQUIRE(!receiver->receiving());
}

TEST_CASE("Receiving a byte off the UartRx Interface","[interface][uart]") {
    /*
     * write a byte to the interface and assert it's driven on the wire
     */

    const int baud = 64;
    const int n_databits = 10; // how many bits make up a dataframe
    const int to_write = 0x55;
    const int data_packet = (to_write | 0x100) << 1;

    int rx_wire = 1; //a mock tx wire for a DUT
    auto receiver = std::make_unique<tb::interface::UartReceiver>(rx_wire, baud); //create a transmitter
    receiver->write_byte((char)to_write); // write b01010101 to the uart
    receiver->pre_tick(); //execute the pre-tick once so advance the state
    //after that pre-tick, the interface should no longer be idle
    REQUIRE(receiver->receiving());


    // expect an alternating series of 0->1 each one occurring baud times
    int expected_data;
    bool bit_transmitted_correctly = true;

    INFO("Receiving data packet 0x" << std::hex << data_packet << " Into UART");
    for(int this_bit=0;this_bit<n_databits;this_bit++){
        expected_data = (data_packet >> this_bit) & 0x01;
        INFO("Transmitting bit " << this_bit << ", Expecting " << expected_data);
        for(int baud_tick=0;baud_tick<baud; baud_tick++){
            INFO("Tick " << baud_tick << " of " << baud);
            receiver->pre_tick();
            bit_transmitted_correctly &= (rx_wire == expected_data);
            CHECK(rx_wire == expected_data);
        }
        REQUIRE(bit_transmitted_correctly);
    }

    //The receiver should be idle after asserting the final byte
    REQUIRE(!receiver->receiving());
}