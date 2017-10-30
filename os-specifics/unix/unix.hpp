#ifndef PROTOMESH_LINUX_HPP
#define PROTOMESH_LINUX_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <utility>
#include <chrono>
#include "api/network.hpp"
#include "api/storage.hpp"
#include "api/time.hpp"

#define STORAGE_PREFIX "/.config/protoMesh"

using boost::asio::ip::udp;

class UnixNetwork : public Network {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    UnixNetwork(std::string multicastGroup, unsigned short port);

    void send(std::vector<uint8_t> message) override;
    int recv(std::vector<uint8_t> *msg, unsigned int timeout_ms) override;
};

class UnixNetworkProvider : public NetworkProvider {
public:
    inline std::vector<NETWORK_T> getAvailableNetworks() override {
        return {
                NETWORK_T(new UnixNetwork(MULTICAST_NETWORK, MULTICAST_PORT))
        };
    }
};

class UnixStorage : public StorageProvider {
public:
    void set(string key, vector<uint8_t> value) override;
    vector<uint8_t> get(string key) override;
    bool has(string key) override;
};

class UnixRelativeTimeProvider : public RelativeTimeProvider {
public:

    REL_TIME_PROV_T toPointer() {
        return REL_TIME_PROV_T(this);
    }

    inline long millis() override {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

#endif //PROTOMESH_LINUX_HPP
