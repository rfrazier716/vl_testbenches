//
// Created by rfraz on 1/6/2021.
//


#include "inc/tcp/udp_server.hpp"
#include <iostream>

udp_server::udp_server(const std::string ip, int listen_port, int transmit_port):
    socket_(context_,udp::endpoint(boost::asio::ip::make_address(ip), listen_port)),
    guard_(context_.get_executor()){
    remote_endpoint_ = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 50001);
}

udp_server::~udp_server() {
    //If that thread has not been joined yet, rejoin it
    if(context_run_thread_.joinable()){
        context_run_thread_.join();
    }
}

void udp_server::start() {
    // run the io_context in its own thread
    start_receive(); //register a receive so we're always listening for input
    context_run_thread_ = std::thread([&]{context_.run();});
}

void udp_server::stop() {
    //reset the guard so the server no longer thinks there's work to do
    guard_.reset(); //close the work guard so the server exits as normal
    context_.stop(); // stop the io_context
    context_run_thread_.join(); //wait for the thread to finish and rejoin
}

void udp_server::start_receive() {
    {
        //Register a receive callback function to the io_context
        socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                [&](boost::system::error_code error, size_t size){handle_receive(error, size);});
    }
}

void udp_server::handle_receive(const boost::system::error_code &error, std::size_t) {
    if(!error){
        rx_queue.push(recv_buffer_[0]); // push the received data onto the buffer
    }
}

void udp_server::handle_send(std::unique_ptr<std::string> &message, const boost::system::error_code &error,
                             std::size_t size) {
    // Pretty sure this is just here to ensure that the lifetime of message is long enough to transmit
}

void udp_server::send(std::string message) {

    auto msg_ptr = std::make_unique<std::string>("Hello World");
    *msg_ptr = message; //
    //*msg_ptr = message;
    socket_.async_send_to(boost::asio::buffer(*msg_ptr),
                          remote_endpoint_,
                          [&](boost::system::error_code error,
                              size_t size){ handle_send(msg_ptr, error, size);});
}
