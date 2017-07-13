#ifndef LUMOS_KEYS_HPP
#define LUMOS_KEYS_HPP

#include <map>

#include "../crypto/crypto.hpp"

class KeyProvider {
    map<PUB_HASH_T, Crypto::asym::PublicKey> keys;
    Crypto::asym::PublicKey master;
public:
    KeyProvider(Crypto::asym::PublicKey masterKey) : master(masterKey) {};

    void pushKey(Crypto::asym::PublicKey key) { this->keys.emplace(key.getHash(), key); }
    void insertKey(PUB_HASH_T hash, Crypto::asym::PublicKey key) { this->keys.emplace(hash, key); }
};

#endif //LUMOS_KEYS_HPP
