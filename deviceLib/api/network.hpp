#ifndef UCL_LIBRARY_H
#define UCL_LIBRARY_H

#include <string>
#include <memory>

#define SOCKET_T std::shared_ptr<Socket>
class Socket {
public:
    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::string *msg, unsigned int timeout_ms)= 0;
};

#define BCAST_SOCKET_T std::shared_ptr<BroadcastSocket>
class BroadcastSocket {
public:
    virtual ~BroadcastSocket()= default;

    virtual void broadcast(std::string message)= 0;

    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::string *msg, unsigned int timeout_ms)= 0;
};

class NetworkProvider {
public:
    // UDP related things
    virtual BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) = 0;

    // TCP related things
    virtual SOCKET_T openChannel(std::string ip) = 0;
};

#ifdef UNIT_TESTING
    class DummyBroadcastSocket : public BroadcastSocket {
    public:
        inline DummyBroadcastSocket(std::string multicastGroup, unsigned short port) {};

        inline void broadcast(std::string message) override {};

        inline void send(std::string ip, unsigned short port, std::string message) override {};

        inline int recv(std::string *msg, unsigned int timeout_ms) override { return 0; };
    };

    class DummyNetworkHandler : public NetworkProvider {
    public:
        // UDP related things
        inline BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) override {
            return BCAST_SOCKET_T(new DummyBroadcastSocket(multicastGroup, port));
        };

        // TCP related things
        inline SOCKET_T openChannel(std::string ip) override { return nullptr; };
    };
#endif

#endif