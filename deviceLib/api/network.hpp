#ifndef UCL_LIBRARY_H
#define UCL_LIBRARY_H

#include <string>
#include <memory>

class Socket {
    virtual void send()= 0;

    virtual std::string recv()= 0;
};

class NetworkHandler {
public:
    // UDP related things
    virtual void setBroadcastTarget(std::string multicastGroup, unsigned short port)= 0;

    virtual void broadcast(std::string message)= 0;

    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::string *msg, unsigned int timeout_ms)= 0;

    // TCP related things
    virtual std::unique_ptr<Socket> openChannel(std::string ip)= 0;
};

#endif