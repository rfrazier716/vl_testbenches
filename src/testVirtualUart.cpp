#include "inc/VirtualUart.hpp"
#include <iostream>
#include <memory> //for smart pointers
#include <boost/range/irange.hpp>

//verilated Tx and Rx blocks used to verify virtual uart
#include "Vuart_rx.h"
#include "Vuart_tx.h"
#include <verilated.h>

#define TICKS_PER_CYCLE 8

#define CLOCK_LINE i_clk //Define what clock line the testbench will toggle
#include "inc/SynchronousTB.hpp"

// Catch Library
#include <catch2/catch.hpp>

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

/*
template <typename WireType>
void transmitByte(std::unique_ptr<VirtualUart<WireType>& vUart, WireType& txWire, int baudCounter, int txByte){
    auto paddedTx = (0x100 | txByte) << 1; //pad with start and stop bits;
    for(int i: boost::irange(10)){
        txWire = (paddedTx >> i) & 0x01; // drive the txWire
        for(int j: boost::irange(baudCounter)){
            vUart->tick(); //tick the server the appropriate number of baud counts
        }
    }
}*/

TEST_CASE("Transmitting a Byte through the vUART","[uart-tb][uart]"){
    //make a new virtualUart
    auto tb = std::make_unique<SyncTB<Vuart_tx>>(50000000, false); //make a uart_tx testbench
    int rxWire = 1;
    auto vUart = std::make_unique<VirtualUart<CData>>(TICKS_PER_CYCLE,(CData&)rxWire, tb->dut->o_tx_w);
    vUart->tick(); //Tick the uart 

    //transmit a series of bytesand make sure they end up on the last transmitted byte
    auto bytesTransmittedProperly = true;
    auto txBufferFilling = true;
    for(int i: boost::irange(0x10)){
        tb->dut->i_data = i; //put the data on the bus
        tb->dut->i_write = 1; //set the write request high
        tb->tick();
        tb->dut->i_write = 0;
        while(tb->dut->o_busy){
            vUart->tick(); //tick the virtual uart
            tb->tick(); //tick the testbench
        }
        bytesTransmittedProperly &= vUart->getLastTxByte() == i;
        txBufferFilling &= vUart->txBuffer[i] == i;
    }
    REQUIRE(bytesTransmittedProperly);
    REQUIRE(txBufferFilling);
}

TEST_CASE("Receiving Bytes through the vUART","[uart-tb][uart]"){
    //make a new virtualUart
    auto tb = std::make_unique<SyncTB<Vuart_rx>>(50000000, false); //make a uart_rx testbench
    int txWire = 1; //make a dummy txWire to connect up to the tx interface, since it's not in this test
    auto vUart = std::make_unique<VirtualUart<CData>>(TICKS_PER_CYCLE,tb->dut->i_rx_w, (CData&)txWire);
    vUart->tick(); //Tick the uart 
    REQUIRE(tb->dut->i_rx_w == 1); //after a tick the uart should have assigned the Rx Wire high

    /////////////////////////////////////////////////////
    // Single Byte Rx case
    ////////////////////////////////////////////////////

    //receive a series of bytes and make sure they end up on on the data line of the uart
    vUart->writeRxBuffer(0xA5); //write a charater to the vUart

    //there should be a 1 tick delay for the virtual uart before it sets the data line low
    vUart->tick();
    REQUIRE(tb->dut->i_rx_w == 1);
    vUart->tick();
    tb->tick();
    REQUIRE(tb->dut->i_rx_w == 0);

    //loop until the vUart is done asserting the bit onto the rx wire
    while(vUart->getRxState() != UARTState::ST_IDLE){
        tb->tick();
        vUart->tick();
    }
    REQUIRE(tb->dut->o_data_w == 0xA5);

    /////////////////////////////////////////////////////
    // Multi Byte Rx case
    ////////////////////////////////////////////////////

    bool receiverFunctional = true;
    for(int i: boost::irange(0xFF)){
        vUart->writeRxBuffer(i);
        vUart->tick();

        while(vUart->getRxState() != UARTState::ST_IDLE){
            tb->tick();
            vUart->tick();
        }
        receiverFunctional &= (tb->dut->o_data_w == i);
    }
    REQUIRE(receiverFunctional);
}