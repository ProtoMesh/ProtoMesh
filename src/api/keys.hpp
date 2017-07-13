#ifndef LUMOS_KEYS_HPP
#define LUMOS_KEYS_HPP

#include <map>

#include "../crypto/crypto.hpp"

class KeyProvider {
    map<PUB_HASH_T, Crypto::asym::KeyPair> keys;
};

#endif //LUMOS_KEYS_HPP
