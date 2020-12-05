//UDP server with basic transmit and receive

#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>
#define BUFFER_DEPTH 1024

using boost::asio::ip::udp;

class UDPServer{
    const int port; //the UDP Port
public:
    UDPServer(int port); //Class constructor
};