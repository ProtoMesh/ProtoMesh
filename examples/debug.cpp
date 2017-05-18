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

    LinuxMCast mcast;
    LinuxUCast ucast;
    LinuxStorage stor;
    Device dev(&ucast, &mcast, &stor);

    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
    Crypto::asym::KeyPair pair2(Crypto::asym::generateKeyPair());

    // Execute this to save some data
//    stor.set("pubkey1", pair.pub.getCompressedString());
//    stor.set("pubkey2", pair2.pub.getCompressedString());
//
//    std::map<PUB_HASH_T, Crypto::asym::PublicKey*> keys;
//    keys[pair.pub.getHash()] = &pair.pub;
//    keys[pair2.pub.getHash()] = &pair2.pub;
//
//    Registry reg("testReg", &keys, &stor);
//    reg.set("someDevice", "someValue", pair);
//    reg.del("someDevice", pair2);
//    reg.set("someDevice", "someEpicValue", pair);

    // Execute this to load it back from the storage
    std::map<PUB_HASH_T, Crypto::asym::PublicKey*> keys2;

    string key1 = stor.get("pubkey1");
    string key2 = stor.get("pubkey2");
    Crypto::asym::PublicKey loadedPubKey1 = Crypto::asym::PublicKey(key1);
    Crypto::asym::PublicKey loadedPubKey2 = Crypto::asym::PublicKey(key2);
    keys2[loadedPubKey1.getHash()] = &loadedPubKey1;
    keys2[loadedPubKey2.getHash()] = &loadedPubKey2;

    Registry reg2("testReg", &keys2, &stor);
    if (!reg2.has("someDevice")) std::cout << "NOTHIN' THERE\n";
    else std::cout << reg2.get("someDevice") << "|done\n";

//    while (interrupted == 0) {
//        dev.tick(1000);
//        sleep(1);
//    }
}