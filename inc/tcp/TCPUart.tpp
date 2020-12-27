#include "inc/tcp/TCPUart.hpp"

template<class T>
TCPUart<T>::TCPUart(int port, int baudCounter):
UartTestBench<T>(baudCounter), io_service(), socket(io_service){
    //Create a socket that represents the connection then wait for a connection
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
    acceptor.accept(socket);
    const char message[] = "Connected to Virtual UART";
    boost::asio::write(socket, boost::asio::buffer(message));
}

template<class T>
void TCPUart<T>::tick(){
    //if there's bytes on the socket read them into the Rx Buffer

    UartTestBench<T>::tick();
    
    //if there's a TXbyte available write it to the socket
    if(!this->vUart->txBuffer.empty())
    {   
        auto& txBuffer = this->vUart->txBuffer;
        boost::asio::write(socket, boost::asio::buffer(txBuffer.linearize(),txBuffer.size()));
        std::cout << int(txBuffer[0]) << std::endl;
        //Clear the buffer now that it's been written to the stream
        txBuffer.clear();
    }
}