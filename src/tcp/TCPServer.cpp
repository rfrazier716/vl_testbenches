#include "inc/tcp/TCPServer.hpp"
#include <assert.h> 



//Creates and binds socket, returning a nonzero means error
int TCPServer::createSocket()
{
    //Create the socket file descriptor
    sockfd = socket(
        AF_INET,        //Use IPv4 Protocol
        SOCK_STREAM,    //Connecting using TCP Protocol
        0);             //Protocol, should be 0

    
    assert(sockfd!=-1); // if the sockfd returns -1 we had an error

    //Create a socket address used to bind
    struct sockaddr_in address;
    address.sin_family = AF_INET; //IPv4 protocol
    address.sin_addr.s_addr = INADDR_ANY; //Using localhost
    address.sin_port = htons(port);  //converts to a host address

    //bind the socket to the created address
    //tie it to the variable bound for return
    bind(sockfd, (struct sockaddr *) &address, sizeof(address));
    listen(sockfd, 3); // listen on the port, max of 3 pending connections
    return 0;
}

int TCPServer::connectToClient(){
    struct sockaddr_in address;
    auto addrlen = sizeof(address);
    clientSocket = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
}

// pushes a char onto the tx buffer
void TCPServer::sendChar(char transmitByte)
{
    txBuffer.push_back(transmitByte);
}

// pushes a char onto the tx buffer
void TCPServer::sendMessage(const char * message, int messageLen)
{
    //push all the messages onto the transmit buffer
    for(int j=0;j<messageLen;j++)
    {
        txBuffer.push_back(*(message+j));
    }
    tick(); //tick the server
}

void TCPServer::sendMessage(const char * message)
{
    //push all the messages onto the transmit buffer
    for(int j=0;j<strlen(message);j++)
    {
        txBuffer.push_back(*(message+j));
    }
    txBuffer.push_back('\r');
    txBuffer.push_back('\n');
    tick(); //tick the server
}

// pops up to maxLength character off of the receive buffer, returns the actual number
// of characters pulled
int TCPServer::getBytes(char * message, int maxLength)
{
    auto bytesPulled = 0;
    while(!rxBuffer.empty() && bytesPulled<maxLength)
    {
        *(message + bytesPulled) = rxBuffer[0];
        rxBuffer.pop_front();
        bytesPulled+=1;
    }
    return bytesPulled;
}


//Ticks the server, updating the tx and rx buffers
void TCPServer::tick()
{
    // if there's bytes to transmit on the txBuffer do so
    if(!txBuffer.empty())
    {
        auto ar = txBuffer.array_one();
        send(clientSocket, ar.first, ar.second, 0); 
        ar = txBuffer.array_two();
        send(clientSocket, ar.first, ar.second, 0);
        txBuffer.clear(); //Clear the buffer
    }

    // if there's bytes to receive pull them into the rxbuffer, but don't overfill
    auto bytesRead = read(clientSocket, rawRxBuffer, rxBuffer.reserve());
    // push the elements that were received into the rxBuffer;
    for(int j=0;j<bytesRead;j++)
    {
        rxBuffer.push_back(*(rawRxBuffer+j));
    }
}

//update the client socket to be either blocking or nonblocking
void TCPServer::setBlocking(bool blocking)
{
    auto flags = fcntl(clientSocket, F_GETFL);
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK) ;
    fcntl(clientSocket, F_SETFL, flags); //update the flags
}

// Class initialization 
TCPServer::TCPServer(int port): port(port), txBuffer(BUFFER_DEPTH), rxBuffer(BUFFER_DEPTH){
    createSocket(); //Create and bind the socket to the port
}

TCPServer::~TCPServer(){
    close(sockfd);
    close(clientSocket);
}