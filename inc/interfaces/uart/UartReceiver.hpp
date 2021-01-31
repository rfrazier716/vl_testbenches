//
// Created by rfraz on 1/31/2021.
//

#ifndef VL_TESTBENCHES_UARTRECEIVER_HPP
#define VL_TESTBENCHES_UARTRECEIVER_HPP

#include "../VirtualInterface.hpp"

namespace tb{
    namespace interface{

        class UartReceiver: protected IOInterface<char>{
        public:
            explicit UartReceiver(int& rx_wire, int baud_clock): rx_wire_(rx_wire), baud_clock_(baud_clock){}
            void write_byte(const char to_write){this->enqueue(to_write);} // add a byte to the uart
            int get_baud_rate() const{return baud_clock_;}

            enum class State {ST_IDLE, ST_DATA_BITS}; //states of the receiver

            void post_tick() override{} //Receiver should have no post-tick operations
            void pre_tick() override; //pre-tick asserts current data bit to rx wire

            bool receiving(){return rx_state_ != State::ST_IDLE;} //if the uart is transmitting
            int get_rx_wire() const {
                return rx_wire_;
            }

        private:
            const int baud_clock_; // how many ticks per bit of data
            int& rx_wire_; //reference to the DUTS rx wire, this is driven by the interface

            State rx_state_=State::ST_IDLE;
            int rx_counter_=0; //a counter for how long the receiver has been in a given state
            int data_packet_ = 0x00; // the data being asserted onto the Uart including header start and stop bits
            int rx_bit_shift_ = 0; //mask for the rx data used in driving the wire
        };

    }
}

#endif //VL_TESTBENCHES_UARTRECEIVER_HPP
