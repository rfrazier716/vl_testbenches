#pragma once //only import once

//Includes for verilator libraries
#include "verilated.h"
#include "verilated_vcd_c.h"

//Include the virtualUART class which will be used as the interface
#include "inc/SynchronousTestBench.hpp" //this testbench inherits from the UartTestBench
#include "inc/VirtualUart.hpp"

#include <memory> //used for smart pointers

template<class T>
class UartTestBench: public SyncTB<T>{
public:
    std::unique_ptr<VirtualUart<CData>> vUart; //the virtualUART port
    UartTestBench(int baudCounter=108); // Class constructor
    ~UartTestBench(); // Class Destructor
    virtual void tick(); // overrride the synchronous testbench tick function
    };

#include "inc/UartTestBench.tpp" //Include the template implemenation file