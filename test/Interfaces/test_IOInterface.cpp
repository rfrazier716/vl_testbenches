//
// Created by rfraz on 1/23/2021.
//


// Catch Library
#include <memory>
#include <catch2/catch.hpp>
#include "interfaces/VirtualInterface.hpp"
#include "interfaces/uart/UartTransmitter.hpp"

// Wrap class definitions in anonymous namespace so they're not accessible to other translation units
namespace {
    class TestInterface : public tb::interface::IOInterface<int> {
        void pre_tick() override {

        }

        void post_tick() override {

        }
    };
}

TEST_CASE("adding/removing a single item to queue","[interface]") {
    /*
     *  Testing adding and removing a single queue element
     */
    auto interface = std::make_unique<TestInterface>();
    auto to_enqueue = 33;

    // push an element to the queue and verify it's no longer empty
    interface->enqueue(to_enqueue);
    REQUIRE(!interface->queue_empty());

    // Get the front value off the queue
    int dequeued;
    interface->dequeue(dequeued);
    //Queue should now be empty
    REQUIRE(interface->queue_empty());
    //the returned value should be what was enqueued
    REQUIRE(dequeued == to_enqueue);

}

TEST_CASE("IOInterface queue unpacked as fifo","[interface]"){
    auto interface = std::make_unique<TestInterface>();
    for(int i=0;i<10;i++){
        interface->enqueue(i);
    }
    int dequeued;
    bool dequeued_in_order = true;
    for(int i=0;i<10;i++){
        interface->dequeue(dequeued);
        dequeued_in_order&= (dequeued == i);
    }
    REQUIRE(dequeued_in_order);
}


