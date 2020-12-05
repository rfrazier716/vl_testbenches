#include "inc/VirtualUart.hpp"
#include <iostream>
#include <memory> //for smart pointers

#include <boost/range/irange.hpp>

// Catch Library
#include <catch2/catch.hpp>

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void transmitByte(std::unique_ptr<VirtualUart>& vUart, int& txWire, int baudCounter, int txByte){
    auto paddedTx = (0x100 | txByte) << 1; //pad with start and stop bits;
    for(int i: boost::irange(10)){
        txWire = (paddedTx >> i) & 0x01; // drive the txWire
        for(int j: boost::irange(baudCounter)){
            vUart->tick(); //tick the server the appropriate number of baud counts
        }
    }
}

TEST_CASE("Transmitting a Byte","[uart-tb][uart]"){
    //make a new virtualUart
    int rxWire;
    int txWire = 1;
    auto baudTicks = 8; // how many ticks per baud cycle
    std::unique_ptr<VirtualUart> vUart = std::make_unique<VirtualUart>(baudTicks, rxWire, txWire);
    vUart->tick(); //Tick the uart 
    REQUIRE(rxWire == 1); //after a tick the uart should have assigned the Rx Wire high

    //transmit a series of bytesand make sure they end up on the last transmitted byte
    auto bytesTransmittedProperly = true;
    for(int i: boost::irange(0xFF)){
        transmitByte(vUart, txWire, baudTicks, i);
        bytesTransmittedProperly &= vUart->getLastTxByte() == i;
    }
}