#include "../deviceLib/api/network.hpp"
#include "../osLib/linux/linux.hpp"
#include "../deviceLib/Device.hpp"
#include "../deviceLib/registry/RegistryEntry.hpp"
#include "../deviceLib/registry/Registry.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted = 1;
}

int main() {
    signal(SIGINT, onInterrupt);

    LinuxStorage ls;
    ls.save("", "");

    LinuxMCast mcast;
    LinuxUCast ucast;
    Device dev(&ucast, &mcast);

    Crypto::asymmetric::KeyPair pair;
    Crypto::asymmetric::KeyPair pair2;

    std::map<PUB_HASH_T, PUBLIC_KEY_T> keys;
    keys[pair.getPublicHash()] = pair.getPublic();
    keys[pair2.getPublicHash()] = pair2.getPublic();

    Registry reg(keys);
    reg.set("someDevice", "someValue", pair);
    reg.del("someDevice", pair2);
    if (!reg.has("someDevice")) std::cout << "NOTHIN' THERE";
    else std::cout << reg.get("someDevice") << "|done";

//    RegistryEntry entry(RegistryEntryType::INSERT, "someDevice", "someValue", pair);
//    std::cout << entry.serialize() << std::endl;
//    RegistryEntry entry2(entry.serialize());
//    std::cout << entry2.serialize() << std::endl;
//
//
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