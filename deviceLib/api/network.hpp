#ifndef UCL_LIBRARY_H
#define UCL_LIBRARY_H

#include <string>
#include <memory>

class Socket {
public:
    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::string *msg, unsigned int timeout_ms)= 0;
};

class BroadcastSocket {
public:
    virtual ~BroadcastSocket()= default;

    virtual void broadcast(std::string message)= 0;

    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::string *msg, unsigned int timeout_ms)= 0;
};

class NetworkHandler {
public:
    // UDP related things
    virtual std::unique_ptr<BroadcastSocket> createBroadcastSocket(std::string multicastGroup, unsigned short port)= 0;

    // TCP related things
    virtual std::unique_ptr<Socket> openChannel(std::string ip)= 0;
};

#ifdef UNIT_TESTING
    class DummyBroadcastSocket : public BroadcastSocket {
    public:
        inline DummyBroadcastSocket(std::string multicastGroup, unsigned short port) {};

        inline void broadcast(std::string message) override {};

        inline void send(std::string ip, unsigned short port, std::string message) override {};

        inline int recv(std::string *msg, unsigned int timeout_ms) override { return 0; };
    };

    class DummyNetworkHandler : public NetworkHandler {
    public:
        // UDP related things
        inline std::unique_ptr<BroadcastSocket> createBroadcastSocket(std::string multicastGroup, unsigned short port) override {
            return std::unique_ptr<BroadcastSocket>(new DummyBroadcastSocket(multicastGroup, port));
        };

        // TCP related things
        inline std::unique_ptr<Socket> openChannel(std::string ip) override { return nullptr; };
    };
#endif

#endif