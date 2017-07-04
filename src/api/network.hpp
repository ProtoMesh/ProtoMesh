#ifndef OPEN_HOME_LIBRARY_H
#define OPEN_HOME_LIBRARY_H

#include <string>
#include <memory>
#include <vector>
#include <const.hpp>
#include <deque>
#include <iostream>

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

    void broadcast(std::string message) {
        std::vector<uint8_t> msg_vec(message.begin(), message.end());
        this->broadcast(msg_vec);
    }

    virtual void broadcast(std::vector<uint8_t> message)= 0;

    virtual void send(std::string ip, unsigned short port, std::string message)= 0;

    virtual int recv(std::vector<uint8_t> *msg, unsigned int timeout_ms)= 0;
};

class NetworkProvider {
public:
    // UDP related things
    virtual BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) = 0;

    // TCP related things
    virtual SOCKET_T openChannel(std::string ip) = 0;
};

#ifdef UNIT_TESTING
    #include <catch.hpp>
    #include <flatbuffers/flatbuffers.h>

    class DummyBroadcastSocket : public BroadcastSocket {
        std::deque<std::vector<uint8_t>> messages;
    public:
        inline DummyBroadcastSocket(std::string multicastGroup, unsigned short port) {};

        inline void broadcast(std::vector<uint8_t> message) override {
            const char* identifier = flatbuffers::GetBufferIdentifier(message.data());
            std::string id(identifier, identifier + flatbuffers::FlatBufferBuilder::kFileIdentifierLength);
            std::cout << "[DUMMY] Broadcasting message (id: '" << id << "')" << std::endl;
            messages.push_back(message);
        };

        inline void send(std::string ip, unsigned short port, std::string message) override {};

        inline int recv(std::vector<uint8_t> *msg, unsigned int timeout_ms) override {
            if (this->messages.size() == 0) return RECV_ERR;

            for (uint8_t byte : this->messages[0]) msg->push_back(byte);

            this->messages.pop_front();
            return RECV_OK;
        };
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