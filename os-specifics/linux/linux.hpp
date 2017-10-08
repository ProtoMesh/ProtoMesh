#ifndef HoMesh_LINUX_HPP
#define HoMesh_LINUX_HPP

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

#define STORAGE_PREFIX "/.config/hoMesh"

using boost::asio::ip::udp;

class LinuxBroadcastSocket : public BroadcastSocket {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    LinuxBroadcastSocket(std::string multicastGroup, unsigned short port);

    void broadcast(std::vector<uint8_t> message) override;

    void send(std::string ip, unsigned short port, std::string message) override;

    int recv(std::vector<uint8_t> *msg, unsigned int timeout_ms) override;
};

class LinuxNetwork : public NetworkProvider {
public:
    // UDP related things
    inline BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) override {
        return BCAST_SOCKET_T(new LinuxBroadcastSocket(multicastGroup, port));
    };

    // TCP related things
    inline SOCKET_T openChannel(std::string ip) override { return nullptr; };
};

class LinuxStorage : public StorageProvider {
public:
    void set(string key, vector<uint8_t> value);
    vector<uint8_t> get(string key);
    bool has(string key);
};

class LinuxRelativeTimeProvider : public RelativeTimeProvider {
public:

    REL_TIME_PROV_T toPointer() {
        return REL_TIME_PROV_T(this);
    }

    inline long millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

#endif //HoMesh_LINUX_HPP
