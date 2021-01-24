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
        public:
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
            bool queue_empty(){return queue_.empty();}
            void enqueue(const PAYLOAD& data);
            void dequeue(PAYLOAD& data);

        private:
            std::queue<PAYLOAD> queue_; // a queue of processed output
        };

        template<class PAYLOAD>
        void IOInterface<PAYLOAD>::enqueue(const PAYLOAD &data) {
            queue_.push(data);
        }

        template<class PAYLOAD>
        void IOInterface<PAYLOAD>::dequeue(PAYLOAD &data) {
            if(!this->queue_empty()) {
                data = queue_.front(); //assign the front to the queue reference
                queue_.pop(); //pop an element off the queue
            }
        }
    }
}

#endif //VL_TESTBENCHES_VIRTUALINTERFACE_H
