#ifndef UCL_LINUX_HPP
#define UCL_LINUX_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <utility>
#include "../../deviceLib/api/network.hpp"
#include "../../deviceLib/api/storage.hpp"

#define STORAGE_PREFIX "/.config/ucl"

using boost::asio::ip::udp;
namespace fs = boost::filesystem;

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

class LinuxNetwork : public NetworkHandler {
public:
    // UDP related things
    inline std::unique_ptr<BroadcastSocket> createBroadcastSocket(std::string multicastGroup, unsigned short port) {
        return std::unique_ptr<BroadcastSocket>(new LinuxBroadcastSocket(multicastGroup, port));
    };

    // TCP related things
    inline std::unique_ptr<Socket> openChannel(std::string ip) override { return nullptr; };
};

class LinuxStorage : public StorageHandler {
public:
    void set(string key, string value);
    string get(string key);
    bool has(string key);
};

#endif //UCL_LINUX_HPP
