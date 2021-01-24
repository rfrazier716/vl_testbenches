//
// Created by rfraz on 1/23/2021.
//

#ifndef VL_TESTBENCHES_UARTTRANSMITTER_HPP
#define VL_TESTBENCHES_UARTTRANSMITTER_HPP

#include "interfaces/VirtualInterface.hpp"


namespace tb{
    namespace interface{

        class UartTransmitter: protected IOInterface<char>{

        protected:
            //transmitter only has a post-tick function, not pre-tick
            void pre_tick() override {}

        public:
            explicit UartTransmitter(int& tx_wire, int baud_clock);
            enum class State {ST_IDLE,ST_DATA_BITS,ST_STOP_BIT}; //states of the transmitter
            void post_tick();


        private:
            State tx_state_;
            int tx_counter_; //a counter for how long the transmitter has been in the state
            char byte_in_progress_='\0'; // holds the current byte being transmitted out
            const int baud_clock_; // how many ticks per bit of data
            int tx_bit_shift_; //shifts the incoming data to the correct bit
            int& tx_wire_; //reference to the DUTS tx wire
        };

    }
}

#endif //VL_TESTBENCHES_UARTTRANSMITTER_HPP
