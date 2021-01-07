//
// Created by rfraz on 1/6/2021.
//

#ifndef WB_UART_UDP_SERVER_HPP
#define WB_UART_UDP_SERVER_HPP

#include <boost/asio.hpp>
#include <memory> //handling smart pointers
#include <string> //std::string library
#include <queue> //for the rx queue
#include <thread> //to run the context in a separate thread


#define BUFFER_SIZE 1024

using boost::asio::ip::udp;

class udp_server
{
public:
    udp_server(const std::string ip= "127.0.0.1", int listen_port = 50000, int transmit_port = 50001); //Class Constructor
    virtual ~udp_server(); //Class Destructor

    void start(); // Start the server
    void stop(); // Stops the server

    void send(std::string message); //transmit a message to the endpoint
    std::queue<std::string> rx_queue; // a queue that holds the received messages

private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error,
                        std::size_t /*bytes_transferred*/);
    void handle_send(std::shared_ptr<std::string> message, const boost::system::error_code &error, std::size_t size);

    boost::asio::io_context context_; //the asio context for the server
    udp::socket socket_; //Socket for the connection
    udp::endpoint remote_endpoint_; //this is the endpoint for the device connecting to the server
    std::vector<std::string> recv_buffer_; //a buffer for received data
    boost::asio::executor_work_guard<decltype(context_.get_executor())> guard_; //guard to ensure context runs

    std::thread context_run_thread_; //thread for io_context.run()

};

#endif //WB_UART_UDP_SERVER_HPP
