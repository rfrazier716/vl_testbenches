/*
 * An Asynchronous TCP server to use with virtual device drivers
 */

#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <atomic>

#include "TCPConnection.hpp"

using boost::asio::ip::tcp;
typedef boost::asio::executor_work_guard<boost::asio::io_context::executor_type> io_guard;


class TCPServer{
    boost::asio::io_context context_; //a unique IO_Context that the server owns
    //a guard will prevent the io_context from exiting when there's no work
    boost::asio::executor_work_guard<decltype(context_.get_executor())> guard_;
    tcp::acceptor acceptor_; // the TCP Server Acceptor
    TCPConnection::pointer connection_; //the TCP Connection -- only supporting one connection for this server
    std::thread context_run_thread_;

    void start_accept(); //Creates socket and initializes an asynchronous accept
    void handle_accept(const boost::system::error_code&); //handle that's called on acceptance

    std::atomic<bool> continue_server_; //flag that decides whether the server should continue or not
    void run(); //private method that is run in a separate thread


public:
    virtual ~TCPServer();
    void start(); // runs the server
    void stop();
    void write_to_connection(std::string &write_message);

    TCPServer(const std::string ip= "127.0.0.1", int port = 8080); //Class Constructor
};