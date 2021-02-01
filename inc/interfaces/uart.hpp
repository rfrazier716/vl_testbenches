//
// Created by rfraz on 1/31/2021.
//

#ifndef VL_TESTBENCHES_UART_HPP
#define VL_TESTBENCHES_UART_HPP

#include "VirtualInterface.hpp"

namespace tb{
    namespace interface{

        class UartLine: protected IOInterface<char>{
        public:
            enum class State {ST_IDLE,ST_DATA_BITS,ST_STOP_BIT}; //states of the UART

            UartLine(int &dataLine, const int baudRate) : baud_rate_(baudRate), data_line_(dataLine) {}
            explicit UartLine(int &dataLine) : data_line_(dataLine), baud_rate_(108) {}

            const int get_baud_rate() const {return baud_rate_;}

        protected:
            State get_state() const {return state_;}
            void set_state(const State state){state_ = state;}

            int& data_line_;

            char byte_in_progress_='\0';
            int bit_shift_=0;
            int state_counter_= 0; //counter to trigger state transitions

        private:
            State state_=State::ST_IDLE;
            const int baud_rate_;
        };

        class UartTransmitter: public UartLine{
        protected:
            //transmitter only has a post-tick function, not pre-tick

        public:
            UartTransmitter(int& tx_line, int baud_rate): UartLine(tx_line, baud_rate){}
            char pop_byte();
            bool byte_available(){return !queue_empty();}

            void pre_tick() override {}
            void post_tick() override;
        };

        class UartReceiver: public UartLine{
        public:
            UartReceiver(int &dataLine, const int baudRate) : UartLine(dataLine, baudRate) {}
            UartReceiver(int &dataLine) : UartLine(dataLine) {}

            void pre_tick() override;
            void post_tick() override {}

            void write_byte(const char to_write){this->enqueue(to_write);} // add a byte to the uart
            bool receiving(){return get_state() != State::ST_IDLE;} //if the uart is transmitting
            int get_rx_wire() const {
                return data_line_;
            }
        private:
            int data_packet_=0;
        };
    }
}
#endif //VL_TESTBENCHES_UART_HPP
