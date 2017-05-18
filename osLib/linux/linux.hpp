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

class LinuxMCast : public MulticastHandler {
    boost::asio::io_service service;
    udp::socket socket;
    udp::endpoint destination;
public:
    LinuxMCast() : service(), socket(service) {};

    void setTarget(std::string multicastGroup, unsigned short port);
    void broadcast(std::string message);
    int receive(std::string* msg, unsigned int timeout_ms);
};

class LinuxUCast : public UnicastHandler {
public:
    bool openChannel(std::string target, unsigned short port) {
        return true;
    };
    void send(std::string message) {};
    void receive(std::string* message, unsigned int timeoutMS) {};
};

class LinuxStorage : public StorageHandler {
public:
    void save(string key, string value);
    string read(string key);
};

#endif //UCL_LINUX_HPP
