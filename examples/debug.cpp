#include <Logger/Logger.hpp>
#include "../os-specifics/linux/linux.hpp"
#include "Network/Manager/NetworkManager.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted++;
}

int main() {
    signal(SIGINT, onInterrupt);

    /// Get the terminal width
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ, &size);

    /// Logger setup
    Logger::setOutputStream(&cout);
    Logger::setOutputWidth(size.ws_col ? size.ws_col : 100);
    Logger::setTimeProvider(make_shared<LinuxRelativeTimeProvider>());
    Logger::setLogLevel(Info);

    /// Print the version
    Logger(Custom, "     Lumos α") << "v0.0.1-6afc6d" << endl;

    NetworkManager networkManager(make_shared<LinuxNetwork>(), make_shared<LinuxStorage>(), make_shared<LinuxRelativeTimeProvider>());

    /// Create a network and join it (normally you'd just join)
    Crypto::asym::KeyPair masterKey = networkManager.createNetwork();
    NETWORK network = networkManager.joinNetwork("someNetwork", masterKey.pub.getCompressed());

    /// Create a node (normally the parameters would be loaded from a file)
    Node testNode(Crypto::UUID(), Crypto::asym::generateKeyPair());

    /// Add a testNode to the network using the master key (normally only done once)
    network->registerNode(testNode.uuid, testNode.serializeForRegistry(), masterKey);

    /// Create a group
    auto gid = network->createGroup(masterKey, {Crypto::UUID(), testNode.uuid});
    Logger(Trace) << "Created group " << gid.unwrap() << endl;

    while (!interrupted) network->tick(1000);

    cout << endl;
    Logger(Error) << "Received interrupt. Goodbye cruel world!" << endl;
}
