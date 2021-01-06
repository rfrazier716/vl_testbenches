//
// Created by rfraz on 1/1/2021.
//

#ifndef WB_UART_TCPCONNECTION_H
#define WB_UART_TCPCONNECTION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <queue>

using boost::asio::ip::tcp;

class TCPConnection: public std::enable_shared_from_this<TCPConnection>{
public:
    std::string test_message = "This is a test\r\n";
    bool connected = false;
    virtual ~TCPConnection(){}
    typedef std::shared_ptr<TCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context){return pointer(new TCPConnection(io_context));}

    tcp::socket socket_; // socket of the connection

    void start(); // Function that is called when the connection is successful
    void write(std::string& write_message); // register an asio write
    void read(); //register an asio read
    void close_connection() { continue_connection_.store(false);}

private:

    std::atomic<bool> continue_connection_;
    //Queues for handling message Rx and Tx
    std::queue<std::string> tx_queue_;
    std::queue<std::string> rx_queue_;

    void check_write(); // check if there's a byte to write and if so set's up an asio_write
    virtual void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);
    virtual void handle_read(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/){}

    // The constructor is private to make sure it's always created as a shared_ptr -- for ASIO
    TCPConnection(boost::asio::io_context& io_context): socket_(io_context) {}


};

#endif //WB_UART_TCPCONNECTION_H
