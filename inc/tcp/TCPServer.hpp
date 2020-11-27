#pragma once // Only execute initialization once


//Necessary includes to get socket working
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// the circular buffer library for TX and RX buffers
#include <boost/circular_buffer.hpp>
#define BUFFER_DEPTH 1024

class TCPServer{
    const int port; // The port for the client to listen on
    int sockfd; // Socket File descriptor (like a file handle)
    int clientSocket; //The socket fd for the first client connection
    char rawRxBuffer[BUFFER_DEPTH]; 
    boost::circular_buffer<char> txBuffer; // Buffer for transmit
    boost::circular_buffer<char> rxBuffer; // Buffer for receive
    int createSocket();
    
public:
    TCPServer(int port); // Class initialization
    ~TCPServer(); //Destructor
    int connectToClient(); // A blocking connection to a client
    void tick(); // tick the server, pull bytes and transmit the buffer
    int getBytes(char * message, int maxLength);
    void sendMessage(const char* message, int messageLen); 
    void sendMessage(const char* message);
    void sendChar(char transmitByte); //Send a byte to the client
    void setBlocking(bool blocking);
};