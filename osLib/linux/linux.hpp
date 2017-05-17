#ifndef UCL_LINUX_HPP
#define UCL_LINUX_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include "../../deviceLib/api/network.hpp"

using boost::asio::ip::udp;

class LinuxMCast : public MulticastHandler {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    LinuxMCast() : service(), socket(service) {};

    void setTarget(std::string multicastGroup, unsigned short port);
    void broadcast(std::string message);
    int receive(std::string* msg, unsigned int timeout_ms);
};

class LinuxUCast : public UnicastHandler {
public:
    bool openChannel(std::string target, unsigned short port) {
        return true;
    };
    void send(std::string message) {};
    void receive(std::string* message, unsigned int timeoutMS) {};
};

#endif //UCL_LINUX_HPP
