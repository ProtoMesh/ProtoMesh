#include "../deviceLib/network.h"
#include "../osLib/linux/linux.hpp"
#include "../deviceLib/Device.hpp"
#include "../deviceLib/Registry.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted = 1;
}

int main() {
    signal(SIGINT, onInterrupt);

    LinuxMCast mcast;
    LinuxUCast ucast;
    Device dev(&ucast, &mcast);

    Crypto::asymmetric::KeyPair pair;
    std::string msg = "test";

    RegistryEntry entry(RegistryEntryType::INSERT, "someDevice", "someValue", pair);
//    std::cout << entry.serialize() << std::endl;
//    RegistryEntry entry2(entry.serialize());
//    std::cout << entry2.serialize() << std::endl;
//
    std::map<PUB_HASH_T, PUBLIC_KEY_T> keys;
    keys[pair.getPublicHash()] = pair.getPublic();

//    RegistryEntry::Verify res = entry.verifySignature(keys);
//    if (res == RegistryEntry::Verify::OK) {
//        printf("VERIFY OK");
//    } else if (res == RegistryEntry::Verify::PubKeyNotFound) {
//        printf("VERIFY FAIL: Pub key not found");
//    } else if (res == RegistryEntry::Verify::SignatureInvalid) {
//        printf("VERIFY FAIL: Signature invalid");
//    }

//    while (interrupted == 0) {
//        dev.tick(1000);
//        sleep(1);
//    }
}