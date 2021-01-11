#pragma once

#include "../testbenches/UartTestBench.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

using boost::asio::ip::tcp;

template<class T>
class TCPUart: public UartTestBench<T>{
    boost::asio::io_service io_service;
    tcp::socket socket;
public:
    TCPUart(int port, int baudCounter = 108);
    void tick();
};

#include "TCPUart.tpp"