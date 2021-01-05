//
// Created by rfraz on 1/1/2021.
//
#include "inc/tcp/TCPConnection.hpp"

void TCPConnection::write(std::string write_message){
    //push message to front of tx_queue
    boost::asio::async_write(socket_, boost::asio::buffer(write_message),
                             boost::bind(&TCPConnection::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void TCPConnection::start(){
    write("Hello TCP World\r\n");
}

void TCPConnection::read(){
    //TODO: write Read Function
}

