#include "../osLib/linux/linux.hpp"
#include "../deviceLib/Device.hpp"

volatile sig_atomic_t interrupted = 0;

void onInterrupt(int) {
    interrupted = 1;
}

int main() {
    signal(SIGINT, onInterrupt);

    LinuxNetwork net;
    LinuxStorage stor;
    LinuxRelativeTimeProvider time;
    Device dev(&net, &stor, time.toPointer());

    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
    Crypto::asym::KeyPair pair2(Crypto::asym::generateKeyPair());

    // Execute this to save some data
    stor.set("pubkey1", pair.pub.getCompressedString());
    stor.set("pubkey2", pair2.pub.getCompressedString());

    std::map<PUB_HASH_T, Crypto::asym::PublicKey *> keys;
    keys[pair.pub.getHash()] = &pair.pub;
    keys[pair2.pub.getHash()] = &pair2.pub;

    Registry reg("testReg", &keys, &stor, &net, time.toPointer());
    reg.clear();
//    reg.set("someDevice", "someValue", pair);
//    reg.del("someDevice", pair2);
//    reg.set("someDevice", "someEpicValue", pair);

//    string firstUUID = reg.entries[0].uuid;

//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"y\",\"parentUUID\":\"" + firstUUID +
//                           "\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//                           false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"z\",\"parentUUID\":\"" + firstUUID +
//                           "\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//                           false);
//    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"x\",\"parentUUID\":\"" + firstUUID +
//                           "\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//                           false);
//    reg.addSerializedEntry(
//            "{\"metadata\":{\"uuid\":\"someintermediate\",\"parentUUID\":\"y\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//            false);
//    reg.addSerializedEntry(
//            "{\"metadata\":{\"uuid\":\"someother\",\"parentUUID\":\"someintermediate\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//            false);
//    reg.addSerializedEntry(
//            "{\"metadata\":{\"uuid\":\"someepic\",\"parentUUID\":\"z\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//            false);
//    reg.addSerializedEntry(
//            "{\"metadata\":{\"uuid\":\"noparents:(\",\"parentUUID\":\"someparentthatdoesn'texist\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}",
//            false);

//    reg.print();

    Registry reg2("testReg", &keys, &stor, &net, time.toPointer());

    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"x\",\"parentUUID\":\"\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"y\",\"parentUUID\":\"x\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"z\",\"parentUUID\":\"y\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"s\",\"parentUUID\":\"z\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"t\",\"parentUUID\":\"s\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"u\",\"parentUUID\":\"t\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg.print();
    std::cout << std::endl;
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"x\",\"parentUUID\":\"\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"y\",\"parentUUID\":\"x\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"z\",\"parentUUID\":\"y\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"s\",\"parentUUID\":\"z\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"t\",\"parentUUID\":\"s\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"u\",\"parentUUID\":\"t\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"a\",\"parentUUID\":\"u\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"b\",\"parentUUID\":\"a\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"c\",\"parentUUID\":\"b\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"d\",\"parentUUID\":\"c\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.addSerializedEntry("{\"metadata\":{\"uuid\":\"e\",\"parentUUID\":\"d\",\"signature\":\"someSignature\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
    reg2.print();


    dev.registries.push_back(reg);
    dev.registries.push_back(reg2);

    // Execute this to load it back from the storage
//    std::map<PUB_HASH_T, Crypto::asym::PublicKey*> keys2;
//
//    string key1 = stor.get("pubkey1");
//    string key2 = stor.get("pubkey2");
//    Crypto::asym::PublicKey loadedPubKey1 = Crypto::asym::PublicKey(key1);
//    Crypto::asym::PublicKey loadedPubKey2 = Crypto::asym::PublicKey(key2);
//    keys2[loadedPubKey1.getHash()] = &loadedPubKey1;
//    keys2[loadedPubKey2.getHash()] = &loadedPubKey2;
//
//    Registry reg2("testReg", &keys2, &stor, &net);
//    if (!reg2.has("someDevice")) std::cout << "NOTHIN' THERE\n";
//    else std::cout << reg2.get("someDevice") << "|done\n";
//
//    reg2.sync();
//    reg2.print();

    while (interrupted == 0) {
        dev.tick(1000);
        reg.sync();
    }
}