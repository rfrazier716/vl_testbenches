#include "inc/tcp/TCPServer.hpp"
#include <string.h>
#include <iostream>


int main(){
    std::cout << "Initializing server and waiting for Client"<<std::endl;
    auto server = new TCPServer(8080);
    auto bytesPulled = 0;
    char message[1024] = {0};
    server->connectToClient();
    server->setBlocking(false);
    std::cout << "Client Connected"<<std::endl;
    
    const char* connectMessage = "Connected to Server\r\n";
    server->sendMessage(connectMessage, strlen(connectMessage));
    server->sendMessage("type \"quit\" to exit");
    bool exit = false;
    while(!exit){
        server->tick();
        bytesPulled = server->getBytes(message, 100);
        message[bytesPulled]='\0'; //put a null terminator down
        if(bytesPulled!=0){
            printf("RX: %s", message);
            exit = (strcmp(message,"quit\r\n") == 0);
        }
    }
    server->sendMessage("Closing Connection");
    std::cout<<"Client disconnected, terminating server" << std::endl;
    delete server; //Delete the server
}