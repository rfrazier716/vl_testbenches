/*
 * An Asynchronous TCP server to use with virtual device drivers
 */

#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <memory>
#include <string>

#include "TCPConnection.hpp"

using boost::asio::ip::tcp;


class TCPServer{
    boost::asio::io_context context_; //a unique IO_Context that the server owns
    tcp::acceptor acceptor_; // the TCP Server Acceptor
    TCPConnection::pointer connection_; //the TCP Connection -- only supporting one connection for this server

    void start_accept(); //Creates socket and initializes an asynchronous accept
    void handle_accept(const boost::system::error_code&);


public:
    void run(); // runs the server
    TCPServer(const std::string ip= "127.0.0.1", int port = 8080); //Class Constructor
};