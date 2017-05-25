#include "linux.hpp"

#define RECV_POOLING_INTERVAL 150000 // 150ms

void LinuxNetwork::setBroadcastTarget(std::string multicastGroup, unsigned short port) {
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

int LinuxNetwork::recv(std::string *msg, unsigned int timeout_ms) {
    boost::asio::ip::udp::endpoint sender;
    std::vector<char> buffer;
    std::size_t bytes_readable = 0;

    int total_time = 0;
    while (!bytes_readable) {
        // Issue command to socket to get number of bytes readable.
        boost::asio::socket_base::bytes_readable num_of_bytes_readable(true);
        socket.io_control(num_of_bytes_readable);

        // Get the value from the command.
        bytes_readable = num_of_bytes_readable.get();

        // If there is no data available, then sleep.
        if (!bytes_readable) {
            usleep(RECV_POOLING_INTERVAL);
        }

        total_time += RECV_POOLING_INTERVAL;
        if (timeout_ms != 0 && total_time > timeout_ms) return 1;
    }

    // Resize the buffer to store all available data.
    buffer.resize(bytes_readable);

    // Read available data.
    socket.receive_from(boost::asio::buffer(buffer, bytes_readable), sender);

    std::string message(buffer.begin(), buffer.end());
    *msg = message;
    return 0;
}

void LinuxNetwork::broadcast(std::string message) {
    socket.send_to(boost::asio::buffer(message), destination);
}

void LinuxNetwork::send(std::string ip, unsigned short port, std::string message) {
    boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    udp::endpoint target(address, port);
    socket.send_to(boost::asio::buffer(message), target);
}

std::string getStorageDirectory() {
    string home = getenv("HOME");
    home += STORAGE_PREFIX;
    fs::create_directories(home);
    return home;
}

void LinuxStorage::set(string key, string value) {
    string home(getStorageDirectory());
    ofstream(home + "/" + key) << value;
}

string LinuxStorage::get(string key) {
    std::stringstream ss;
    string home(getStorageDirectory());

    ss << ifstream(home + "/" + key).rdbuf();
    return ss.str();
}

bool LinuxStorage::has(string key) {
    string home(getStorageDirectory());
    return fs::exists(home + "/" + key);
}
