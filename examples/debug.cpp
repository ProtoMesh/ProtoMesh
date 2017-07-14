#include "../os-specifics/linux/linux.hpp"
#include "Network/NetworkManager.hpp"
#include "Node/Node.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted++;
}

//NETWORK createOrJoinNetwork(NetworkManager networkManager) {
//    if (networkManager.lastJoinedAvailable()) {
//        /// Join the previous network
//        NETWORK network = networkManager.joinLastNetwork();
//        return network;
//    }
//    /// No previous network available so create a new one and join it.
//    Crypto::asym::KeyPair netKeyPair = networkManager.createNetwork();
//    NETWORK network = networkManager.joinNetwork("someNetwork", netKeyPair.pub.getCompressed());
//    return network;
//}

int main() {
    signal(SIGINT, onInterrupt);

    NetworkManager networkManager(make_shared<LinuxNetwork>(), make_shared<LinuxStorage>(), make_shared<LinuxRelativeTimeProvider>());
//    NETWORK network = createOrJoinNetwork(networkManager);

    /// Create a network and join it
    Crypto::asym::KeyPair masterKey = networkManager.createNetwork();
    NETWORK network = networkManager.joinNetwork("someNetwork", masterKey.pub.getCompressed());

    /// Add a testNode to the network using the master key
    Node testNode(Crypto::UUID(), Crypto::asym::generateKeyPair());
    network->registerNode(testNode.uuid, testNode.serializeForRegistry(), masterKey);

//    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
    while (!interrupted) {
        network->tick(1000);
//        if (interrupted % 2 != 0) {
//            cout << endl << "Added entry." << endl;
//            auto sp = network->registries.find("network::nodes")->second;
//            sp->set("test", {1,2,3,4}, pair);
//            interrupted++;
//        }
    }
}
