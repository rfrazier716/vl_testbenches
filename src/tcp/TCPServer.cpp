#include "inc/tcp/TCPServer.hpp"

TCPServer::TCPServer(const std::string ip, int port):
    acceptor_(context_, tcp::endpoint(boost::asio::ip::make_address(ip), port)){
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
    context_.run();
}
