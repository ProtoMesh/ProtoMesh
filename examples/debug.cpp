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
    if (!reg.has("someDevice")) std::cout << "NOTHIN' THERE\n";
    else std::cout << reg.get("someDevice") << "|done\n";

    RegistryEntry entry(RegistryEntryType::UPSERT, "someDevice", "someValue", pair);
    RegistryEntry entry2(entry.serialize());
    if (entry.serialize() != entry2.serialize()) printf("NOT EQUAL");


    RegistryEntry::Verify res = entry.verifySignature(keys);
    if (res == RegistryEntry::Verify::OK) {
        printf("VERIFY OK");
    } else if (res == RegistryEntry::Verify::PubKeyNotFound) {
        printf("VERIFY FAIL: Pub key not found");
    } else if (res == RegistryEntry::Verify::SignatureInvalid) {
        printf("VERIFY FAIL: Signature invalid");
    }

//    while (interrupted == 0) {
//        dev.tick(1000);
//        sleep(1);
//    }
}