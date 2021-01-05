//
// Created by rfraz on 1/1/2021.
//
#include "inc/tcp/TCPConnection.hpp"

void TCPConnection::write(std::string write_message){
    //push message to front of tx_queue
    tx_queue_.push(write_message);
}

void TCPConnection::start(){
    check_write(); // call once explicitly to add it as a handler
}

void TCPConnection::check_write() {
    // if the TX Queue isn't empty, pop a byte off of it and add async_write work
    if(!tx_queue_.empty()){
        //set up an ASIO message transmit
        boost::asio::async_write(socket_, boost::asio::buffer(tx_queue_.front()),
                                 [self = shared_from_this()](boost::system::error_code error, size_t size){
            self->handle_write(error,size);});
        tx_queue_.pop(); // pop the front off the queue
    }
    // if the TX queue is empty, re-register the even with the io_context executor
    else{
        if(continue_connection_) {
            boost::asio::post(socket_.get_executor(),
                              boost::bind(&TCPConnection::check_write, shared_from_this()));
        }
    }
}

void TCPConnection::read(){
    //TODO: write Read Function
}

void TCPConnection::handle_write(const boost::system::error_code &error, size_t size) {
    //the write handler should re-register the check_write handle
    check_write(); //go back to check write to see if there's more data to transmit
}
