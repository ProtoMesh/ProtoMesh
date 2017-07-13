#include "../os-specifics/linux/linux.hpp"
#include "Device/Device.hpp"
#include "Network/NetworkManager.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted++;
}

Network createOrJoinNetwork(NetworkManager networkManager) {
    if (networkManager.lastJoinedAvailable()) {
        // Join the previous network
        Network network = networkManager.joinLastNetwork();
        return network;
    }
    // No previous network available so create a new one and join it.
    Crypto::asym::KeyPair netKeyPair = networkManager.createNetwork();
    Network network = networkManager.joinNetwork("someNetwork", netKeyPair.pub.getCompressed());
    return network;
}

int main() {
    signal(SIGINT, onInterrupt);

    NetworkManager networkManager(make_shared<LinuxNetwork>(), make_shared<LinuxStorage>(), make_shared<LinuxRelativeTimeProvider>());
//    Network network = createOrJoinNetwork(networkManager);
    Crypto::asym::KeyPair netKeyPair = networkManager.createNetwork();
    Network network = networkManager.joinNetwork("someNetwork", netKeyPair.pub.getCompressed());

//    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
    Crypto::asym::KeyPair pair = netKeyPair;
    while (interrupted < 11) {
        network.tick(1000);
        if (interrupted % 2 != 0) {
            cout << endl << "Added entry." << endl;
            auto sp = network.registries.find("network::nodes")->second;
            sp->set("test", {1,2,3,4}, pair);
            interrupted++;
        }
    }
}
