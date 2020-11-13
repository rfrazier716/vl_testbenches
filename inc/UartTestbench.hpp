#pragma once

#include "inc/SynchronousTB.hpp"

// a template class for a UART 
template <class T>
class UartTestBench: public SyncTB<T>{

};