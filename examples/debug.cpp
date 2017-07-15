#include "../os-specifics/linux/linux.hpp"
#include "Network/NetworkManager.hpp"
#include "Node/Node.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted++;
}

int main() {
    signal(SIGINT, onInterrupt);

    NetworkManager networkManager(make_shared<LinuxNetwork>(), make_shared<LinuxStorage>(), make_shared<LinuxRelativeTimeProvider>());

    /// Create a network and join it (normally you'd just join)
    Crypto::asym::KeyPair masterKey = networkManager.createNetwork();
    NETWORK network = networkManager.joinNetwork("someNetwork", masterKey.pub.getCompressed());

    /// Create a node (normally the parameters would be loaded from a file)
    Node testNode(Crypto::UUID(), Crypto::asym::generateKeyPair());
    /// Tell the node which network to use
    testNode.registerAt(network);

    /// Add a testNode to the network using the master key (normally only done once)
    network->registerNode(testNode.uuid, testNode.serializeForRegistry(), masterKey);

    /// Create a group (passing a UUID would attempt to load the group first and create it if that fails)
    Group newGroup(Group::createGroup(network));

    while (!interrupted) network->tick(1000);
}
