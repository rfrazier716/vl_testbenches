#include "inc/tcp/TCPServer.hpp"

TCPServer::TCPServer(const std::string ip, int port):
    acceptor_(context_, tcp::endpoint(boost::asio::ip::make_address(ip), port)),
    guard_(context_.get_executor())
    {
    start_accept(); // Make acceptor Object and connect
}

void TCPServer::start_accept(){
    connection_ = TCPConnection::create(context_);
    acceptor_.async_accept(connection_->socket_,
                           boost::bind(&TCPServer::handle_accept,
                                       this,
                                       boost::asio::placeholders::error
                                       ));
}

void TCPServer::handle_accept(const boost::system::error_code& error) {
    if(!error){
        connection_->start();
    }
}

void TCPServer::run(){
    while(continue_server_.load()) {
        context_.run();
        context_.restart();
    }
}

void TCPServer::write_to_connection(std::string &write_message) {
    connection_->write(write_message);
}

void TCPServer::start() {
    //make a new thread to run the run method
    continue_server_.store(true);
    context_run_thread_ = std::thread([&] { context_.run(); });
    while(!connection_->connected); // loop until a connection is made
    connection_->write(connection_->test_message);
}

void TCPServer::stop() {
    guard_.reset(); //close the work guard so the server exits as normal
    context_run_thread_.join(); //wait for the thread to finish
}

TCPServer::~TCPServer(){
    if(context_run_thread_.joinable()) context_run_thread_.join();
}
