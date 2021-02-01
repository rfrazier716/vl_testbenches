//
// Created by rfraz on 1/31/2021.
//

#include "interfaces/uart.hpp"

void tb::interface::UartTransmitter::post_tick() {
    State next_state;
    switch(this->get_state()){
        case State::ST_IDLE:
            state_counter_ = this->get_baud_rate()*1.5;
            bit_shift_ = 0;
            // if the output wire is driven low start the capture
            next_state = (data_line_ == 0) ? State::ST_DATA_BITS : State::ST_IDLE;
            break;

        case State::ST_DATA_BITS:
            state_counter_--;//decrement the counter
            //if the counter is zero latch the data bit and go to the next
            if(state_counter_ == 0){

                state_counter_ = this->get_baud_rate(); // Reset the baud counter
                byte_in_progress_ |= (data_line_ << bit_shift_); //latch the date being output by the transmitter
                bit_shift_++; // increment the bit shift
                //if we've latched 8 bits go to the stop bit
                if(bit_shift_ == 8)
                {
                    state_counter_ = this->get_baud_rate()/2; //the stop bit baud counter is halved
                    next_state = State::ST_STOP_BIT; //move to the stop bit counter
                }
            }
            break;
        case State::ST_STOP_BIT:
            state_counter_--; //decrement teh counter
            if(state_counter_ == 0){
                enqueue(byte_in_progress_); //put the completed byte onto the queue
                byte_in_progress_ = '\0'; //clear the byte in progress
                next_state = State::ST_IDLE; //advance to the idle state
            }
            break;
    }
    set_state(next_state);

}

char tb::interface::UartTransmitter::pop_byte() {
    char tx_byte = '\0';
    dequeue(tx_byte);
    return tx_byte;
}

void tb::interface::UartReceiver::pre_tick() {
    State next_state;
    switch(get_state()){
        case State::ST_IDLE:
            state_counter_ = get_baud_rate(); //reset the state_counter_
            bit_shift_ = 0; //the current bit-shift is zero
            data_line_ = 1; // if there is not data incoming, make sure the rx wire is pulled high

            // if we are able to pull data off the rx buffer go into the data state
            if(!this->queue_empty()){
                char to_write; //memory location to pull the data to write off of the queue
                this->dequeue(to_write);
                data_packet_ = (int(to_write) | 0x100) <<1; // add start and stop bits
                next_state = State::ST_DATA_BITS; //advance the state
            }
            break;

        case State::ST_DATA_BITS:
            //assert a value onto the rx wire
            state_counter_--; //Decrease the counter
            data_line_ = (data_packet_ >> bit_shift_) & 0x01; //put the relevant part of data on the Rx Line

            // if the baudCounter zeroed out advance the rx_bit_shift_ and reset the counter
            if(state_counter_ == 0)
            {
                state_counter_ = get_baud_rate(); // Reset the Rx Counter
                bit_shift_++; //increment the bit shift
            }
            // if 10 bytes have been received go back into the idle state
            next_state = (bit_shift_ == 10) ? State::ST_IDLE : State::ST_DATA_BITS;
            break;
    }
    set_state(next_state);
}

