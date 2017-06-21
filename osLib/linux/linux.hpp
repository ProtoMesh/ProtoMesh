#ifndef UCL_LINUX_HPP
#define UCL_LINUX_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <utility>
#include <chrono>
#include "../../deviceLib/api/network.hpp"
#include "../../deviceLib/api/storage.hpp"
#include "../../deviceLib/api/time.hpp"

#define STORAGE_PREFIX "/.config/ucl"

using boost::asio::ip::udp;

class LinuxBroadcastSocket : public BroadcastSocket {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    LinuxBroadcastSocket(std::string multicastGroup, unsigned short port);

    void broadcast(std::string message) override;

    void send(std::string ip, unsigned short port, std::string message) override;

    int recv(std::string *msg, unsigned int timeout_ms) override;
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
    void set(string key, string value);
    string get(string key);
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

#endif //UCL_LINUX_HPP
