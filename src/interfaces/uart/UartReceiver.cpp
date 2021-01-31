//
// Created by rfraz on 1/31/2021.
//
#include "interfaces/uart/UartReceiver.hpp"

void tb::interface::UartReceiver::pre_tick() {
    switch(rx_state_){
        case State::ST_IDLE:
            rx_counter_ = baud_clock_; //reset the rx_counter_
            rx_bit_shift_ = 0; //the current bit-shift is zero

            rx_wire_ = 1; // if there is not data incoming, make sure the rx wire is pulled high

            // if we are able to pull data off the rx buffer go into the data state
            if(!this->queue_empty()){
                char to_write; //memory location to pull the data to write off of the queue
                this->dequeue(to_write);
                data_packet_ = (int(to_write) | 0x100) <<1; // add start and stop bits
                rx_state_ = State::ST_DATA_BITS; //advance the state
            }
            break;

        case State::ST_DATA_BITS:

            //assert a value onto the rx wire
            rx_counter_--; //Decrease the counter
            rx_wire_ = (data_packet_ >> rx_bit_shift_) & 0x01; //put the relevant part of data on the Rx Line

            // if the baudCounter zeroed out advance the rx_bit_shift_ and reset the counter
            if(rx_counter_ == 0)
            {
                rx_counter_ = baud_clock_; // Reset the Rx Counter
                rx_bit_shift_++; //increment the bit shift
            }
            // if 10 bytes have been received go back into the idle state
            rx_state_ = (rx_bit_shift_ == 10) ? State::ST_IDLE : State::ST_DATA_BITS;
            break;
    }
}
