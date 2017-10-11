#include <fstream>
#include "unix.hpp"

#define RECV_POOLING_INTERVAL 150000 // 150ms

UnixNetwork::UnixNetwork(std::string multicastGroup, unsigned short port) : Network(NetworkType::WIRED, NetworkAccess::FREE), service(), socket(service) {
    boost::asio::ip::address address = boost::asio::ip::address::from_string(multicastGroup);

    // Open up the socket
    socket.open(boost::asio::ip::udp::v4());

    // Allow other processes to reuse the address, permitting other processes on
    // the same machine to use the multicast address.
    socket.set_option(udp::socket::reuse_address(true));

    // Guarantee the loopback is enabled so that multiple processes on the same
    // machine can receive data that originates from the same socket.
    socket.set_option(boost::asio::ip::multicast::enable_loopback(true));
    socket.bind(udp::endpoint(boost::asio::ip::address_v4::any(), port));

    destination = udp::endpoint(address, port);

    // Join group.
    namespace ip = boost::asio::ip;
    socket.set_option(ip::multicast::join_group(address));
}

int UnixNetwork::recv(std::vector<uint8_t> *buffer, unsigned int timeout_ms) {
    boost::asio::ip::udp::endpoint sender;
//    std::vector<char> buffer;
    std::size_t bytes_readable = 0;

    unsigned int total_time = 0;
    while (!bytes_readable) {
        // Issue command to socket to get number of bytes readable.
        boost::asio::socket_base::bytes_readable num_of_bytes_readable(true);
        socket.io_control(num_of_bytes_readable);

        // Get the value from the command.
        bytes_readable = num_of_bytes_readable.get();

        // If there is no data available, then sleep.
        if (!bytes_readable) {
            usleep(RECV_POOLING_INTERVAL);
            total_time += RECV_POOLING_INTERVAL;
            if (timeout_ms != 0 && total_time > timeout_ms) return RECV_ERR;
        }
    }

    // Resize the buffer to store all available data.
    (*buffer).resize(bytes_readable);

    // Read available data.
    socket.receive_from(boost::asio::buffer((*buffer), bytes_readable), sender);

    return RECV_OK;
}

void UnixNetwork::send(std::vector<uint8_t> message) {
    socket.send_to(boost::asio::buffer(message), destination);
}

std::string getStorageDirectory() {
    string home = getenv("HOME");
    home += STORAGE_PREFIX;

    struct stat st = {0};
    if (stat(home.c_str(), &st) == -1) mkdir(home.c_str(), 0700);
    return home;
}

void UnixStorage::set(string key, vector<uint8_t> value) {
    ofstream file(getStorageDirectory() + '/' + key, ios::out | ios::binary);
    if (!file.is_open()) return;

    file.write((const char *) value.data(), value.size());

    file.close();
}

vector<uint8_t> UnixStorage::get(string key) {
    // open the file:
    std::ifstream file(getStorageDirectory() + '/' + key, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<uint8_t > vec;
    vec.reserve((unsigned long) fileSize);

    // read the data:
    vec.insert(vec.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());

    return vec;
//    ifstream file;
//    file.open(getStorageDirectory() + '/' + key);
//    if (!file.is_open()) return {};
//    string line;
//    getline(file,line);
//    return line;
}

bool UnixStorage::has(string key) {
    string target(getStorageDirectory() + '/' + key);

    struct stat buffer;
    return (stat (target.c_str(), &buffer) == 0);
}
