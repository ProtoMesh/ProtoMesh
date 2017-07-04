#include "../os-specifics/linux/linux.hpp"
#include "Device/Device.hpp"
#include "Network/NetworkManager.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted = 1;
}

int main() {
    signal(SIGINT, onInterrupt);

    LinuxNetwork net;
    LinuxStorage stor;
    LinuxRelativeTimeProvider time;
    auto timePtr = time.toPointer();

    NetworkManager networkManager(&net, &stor, timePtr);
    if (networkManager.lastJoinedAvailable()) {
        Network network = networkManager.joinLastNetwork();
    } else {
        Crypto::asym::KeyPair netKeyPair = networkManager.createNetwork();
        Network network = networkManager.joinNetwork("someNetwork", netKeyPair.pub.getCompressed());
    }

    Device dev(&net, &stor, timePtr);

    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
//    Crypto::asym::KeyPair pair2(Crypto::asym::generateKeyPair());
//
//    std::map<PUB_HASH_T, Crypto::asym::PublicKey *> keys;
//    keys[pair.pub.getHash()] = &pair.pub;
//    keys[pair2.pub.getHash()] = &pair2.pub;
//
    Registry<string> reg("testReg", &stor, &net, timePtr);
    cout << reg.has("test") << endl;
    cout << reg.get("test") << endl;
//    reg.clear();
    reg.set("test", "HELLO WORLD! IT WORKED3!", pair);
    reg.sync();

//
//    Registry<string> reg2("testReg", &stor, &net, timePtr);
//    reg2.clear();
//
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"x\",\"parentUUID\":\"\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"y\",\"parentUUID\":\"x\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"z\",\"parentUUID\":\"y\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"s\",\"parentUUID\":\"z\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"t\",\"parentUUID\":\"s\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"u\",\"parentUUID\":\"t\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg.print();
//    std::cout << std::endl;
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"x\",\"parentUUID\":\"\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"y\",\"parentUUID\":\"x\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"z\",\"parentUUID\":\"y\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"s\",\"parentUUID\":\"z\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"t\",\"parentUUID\":\"s\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"u\",\"parentUUID\":\"t\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"a\",\"parentUUID\":\"u\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"b\",\"parentUUID\":\"a\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"c\",\"parentUUID\":\"b\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"d\",\"parentUUID\":\"c\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
//    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"e\",\"parentUUID\":\"d\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someLastValueThatDefinitelyComesLast\"}}", false);
//    reg2.print();

//    dev.registries.push_back(reg);
//    dev.registries.push_back(reg2);

    while (interrupted == 0) {
        dev.tick(1000);
        reg.sync();
    }

//    dev.print();
}
