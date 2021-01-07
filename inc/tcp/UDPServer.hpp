//
// Created by rfraz on 1/6/2021.
//

#ifndef WB_UART_UDPSERVER_HPP
#define WB_UART_UDPSERVER_HPP

#include <boost/asio.hpp>
#include <memory> //handling smart pointers
#include <string> //std::string library
#include <queue> //for the rx queue


#define BUFFER_SIZE 1024

using boost::asio::ip::udp;

class UDPServer
{
public:
    UDPServer(const std::string ip= "127.0.0.1", int port = 50000); //Class Constructor

private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error,
                        std::size_t /*bytes_transferred*/);
    void handle_send(boost::shared_ptr<std::string> /*message*/,
                     const boost::system::error_code& /*error*/,
                     std::size_t /*bytes_transferred*/);

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::queue<std::string> recv_buffer_; //a buffer for received data
};

#endif //WB_UART_UDPSERVER_HPP
