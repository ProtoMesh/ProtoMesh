#ifndef HoMesh_LIBRARY_H
#define HoMesh_LIBRARY_H

#include <string>
#include <memory>
#include <vector>
#include <const.hpp>
#include <deque>
#include <iostream>

enum NetworkType {
    BLUETOOTH,
    WIFI,
    WIRED,
    NATIVE,
    OTHER
};

enum NetworkAccess {
    RESTRICTED,
    METERED,
    FREE
};

#define NETWORK_T std::shared_ptr<Network>
class Network {
public:
    NetworkType type = NetworkType::NATIVE;
    NetworkAccess access = NetworkAccess::FREE;

    Network(NetworkType type, NetworkAccess access) : type(type), access(access) {}
    ~Network() = default;

    virtual void send(std::vector<uint8_t> message)= 0;
    virtual int recv(std::vector<uint8_t> *buffer, unsigned int timeout_ms)= 0;
};

class NetworkProvider {
public:
    ~NetworkProvider() = default;

    virtual std::vector<NETWORK_T> getAvailableNetworks() = 0;
};

//#ifdef UNIT_TESTING
////#define DEBUG 1
//    #include <catch.hpp>
//    #include <flatbuffers/flatbuffers.h>
//
//    class DummyBroadcastSocket : public BroadcastSocket {
//        std::deque<std::vector<uint8_t>> messages;
//    public:
//        inline DummyBroadcastSocket(std::string multicastGroup, unsigned short port) {};
//
//        inline void broadcast(std::vector<uint8_t> message) override {
//#ifdef DEBUG
//            const char* identifier = flatbuffers::GetBufferIdentifier(message.data());
//            std::string id(identifier, identifier + flatbuffers::FlatBufferBuilder::kFileIdentifierLength);
//            std::cout << "[DUMMY] Broadcasting message (id: '" << id << "')" << std::endl;
//#endif
//            messages.push_back(message);
//        };
//
//        inline void send(std::string ip, unsigned short port, std::string message) override {};
//
//        inline int recv(std::vector<uint8_t> *msg, unsigned int timeout_ms) override {
//            if (this->messages.size() == 0) return RECV_ERR;
//
//            for (uint8_t byte : this->messages[0]) msg->push_back(byte);
//
//            this->messages.pop_front();
//            return RECV_OK;
//        };
//    };
//
//    class DummyNetworkHandler : public NetworkProvider {
//    public:
//        // UDP related things
//        inline BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) override {
//            return BCAST_SOCKET_T(new DummyBroadcastSocket(multicastGroup, port));
//        };
//
//        // TCP related things
//        inline SOCKET_T openChannel(std::string ip) override { return nullptr; };
//    };
//#endif

#endif