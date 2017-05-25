#ifndef UCL_LINUX_HPP
#define UCL_LINUX_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include "../../deviceLib/api/network.hpp"
#include "../../deviceLib/api/storage.hpp"

#define STORAGE_PREFIX "/.config/ucl"

using boost::asio::ip::udp;
namespace fs = boost::filesystem;

class LinuxNetwork : public NetworkHandler {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    inline LinuxNetwork() : service(), socket(service) {};

    // UDP related things
    void setBroadcastTarget(std::string multicastGroup, unsigned short port);

    void broadcast(std::string message);

    void send(std::string ip, unsigned short port, std::string message);

    int recv(std::string *msg, unsigned int timeout_ms);

    // TCP related things
    inline std::unique_ptr<Socket> openChannel(std::string ip) { return nullptr; };
};

class LinuxStorage : public StorageHandler {
public:
    void set(string key, string value);
    string get(string key);
    bool has(string key);
};

#endif //UCL_LINUX_HPP
