//
// Created by rfraz on 1/21/2021.
//

#ifndef VL_TESTBENCHES_VIRTUALINTERFACE_H
#define VL_TESTBENCHES_VIRTUALINTERFACE_H

#include <queue>

namespace tb{
    namespace interface {

        class BaseInterface {
            //virtual interface is an abstract class for interfaces, must have a pre_tick and post_tick function
            virtual void pre_tick()=0;
            virtual void post_tick()=0;
        };


        template <class PAYLOAD>
        class IOInterface: public BaseInterface{
            /*
             * An IO interface is an abstract base class used for input and output interfaces, they have a reference to
             * the dut BUS that is sampled/driven to generate data
             */
        public:
            IOInterface(int& bus_wire);
        private:
            int& bus_; //The bus being sampled
            std::queue<PAYLOAD> payload_; // a queue of processed output
        };

        template<class PAYLOAD>
        IOInterface<PAYLOAD>::IOInterface(int &bus_wire) {
            bus_ = bus_wire;
        }

        template <class PAYLOAD>
        class OutputInterface: public IOInterface<PAYLOAD>{
        public:
        };
    }

}

#endif //VL_TESTBENCHES_VIRTUALINTERFACE_H
