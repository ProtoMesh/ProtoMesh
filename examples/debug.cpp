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

    LinuxNetwork net;
    LinuxStorage stor;
    LinuxRelativeTimeProvider time;
    auto timePtr = time.toPointer(); // TODO This causes problems on deallocation!

    NetworkManager networkManager(&net, &stor, timePtr);
    Network network = createOrJoinNetwork(networkManager);

    while (interrupted < 11) {
        network.tick(1000);
        if (interrupted % 2 != 0) {
            cout << endl << "Added entry." << endl;
            auto sp = network.registries.find("groups")->second;
            sp->set("test", {1,2,3,4}, Crypto::asym::generateKeyPair());
            interrupted++;
        }
//        else { interrupted++; }
    }
}
