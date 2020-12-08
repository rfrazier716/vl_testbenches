#include "inc/UartTestBench.hpp"

// Execute a clock cycle and update variables
// you need the eval at start and end, according to zipCPU
template<class T>
void UartTestBench<T>::tick(){
    // on a tick of the Uart test Bench the following should happen
    // Run the rxDrive state machine - this will set inputs to the dut
    // run the SyncTB tick function
    // run the txLatch state machine -- this captures data onto the Vuart from the dut

    vUart->driveRxWire(); //assert signals onto teh RXWire before ticking the dut
    SyncTB<T>::tick(); //call the synchronous Test Bench's tick function
    vUart->captureTxWire(); //capture the new Tx Data
}


template<class T>
UartTestBench<T>::UartTestBench(int baudCounter): SyncTB<T>(50000000, false){
    //attach a virtul UART to the test bench
    vUart = std::make_unique<VirtualUart<CData>>(baudCounter, this->dut->i_rx_w, this->dut->o_tx_w);
}

template<class T>
UartTestBench<T>::~UartTestBench(){
    //nothing goes here right now
    }