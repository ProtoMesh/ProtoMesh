#ifndef UCL_LIBRARY_H
#define UCL_LIBRARY_H

#include <string>

class MulticastHandler {
public:
    virtual void setTarget(std::string multicastGroup, unsigned short port)= 0;
    virtual void broadcast(std::string message)= 0;
    virtual int receive(std::string* msg, unsigned int timeout_ms)= 0;
};

class UnicastHandler {
    virtual bool openChannel(std::string target, unsigned short port)= 0;
    virtual void send(std::string message)= 0;
    virtual void receive(std::string* message, unsigned int timeoutMS)= 0;
};

#endif