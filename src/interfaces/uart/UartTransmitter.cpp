//
// Created by rfraz on 1/23/2021.
//

#include "interfaces/uart/UartTransmitter.hpp"


tb::interface::UartTransmitter::UartTransmitter(int &tx_wire, int baud_clock):
    baud_clock_(baud_clock), tx_wire_(tx_wire){

}

void tb::interface::UartTransmitter::post_tick() {
    switch(tx_state_){
        case State::ST_IDLE:
            tx_counter_ = baud_clock_*1.5;
            tx_bit_shift_ = 0;
            // if the output wire is driven low start the capture
            tx_state_ = (tx_wire_ == 0) ? State::ST_DATA_BITS : State::ST_IDLE;
            break;
            
        case State::ST_DATA_BITS:
            tx_counter_--;//decrement the counter
            //if the counter is zero latch the data bit and go to the next
            if(tx_counter_ == 0){

                tx_counter_ = baud_clock_; // Reset the baud counter
                byte_in_progress_ |= (tx_wire_ << tx_bit_shift_); //latch the date being output by the transmitter
                tx_bit_shift_++; // increment the bit shift
                //if we've latched 8 bits go to the stop bit
                if(tx_bit_shift_ == 8)
                {
                    tx_counter_ = baud_clock_/2; //the stop bit baud counter is halved
                    tx_state_ = State::ST_STOP_BIT; //move to the stop bit counter
                }
            }
            break;
        case State::ST_STOP_BIT:
            tx_counter_--; //decrement teh counter
            if(tx_counter_ == 0){
                enqueue(byte_in_progress_); //put the completed byte onto the queue
                byte_in_progress_ = '\0'; //clear the byte in progress
                tx_state_ = State::ST_IDLE; //advance to the idle state
            }
            break;
    }

}

char tb::interface::UartTransmitter::pop_byte() {
    char tx_byte = '\0';
    dequeue(tx_byte);
    return tx_byte;
}
