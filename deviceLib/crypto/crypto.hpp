#ifndef UCL_CRYPTO_HPP
#define UCL_CRYPTO_HPP

#include <random>
#include <array>
#include <vector>
#include <algorithm>

#include "sha512.hpp"
#include "ecc/uECC.h"

using namespace std;

// Defining ECC key sizes
#define PRIV_KEY_SIZE 32
#define PUB_KEY_SIZE 64
#define COMPRESSED_PUB_KEY_SIZE PRIV_KEY_SIZE + 1
#define SIGNATURE_SIZE PUB_KEY_SIZE

// Define the size in bytes of the short hash to identify a public key
// Is required to be dividable by two.
#define PUB_HASH_SIZE PUB_KEY_SIZE / 4
#define PUB_HASH_T array<char, PUB_HASH_SIZE> // First PUB_HASH_SIZE characters of the HASH of the hex representation of the public key

// Defining cryptography types
#define PUBLIC_KEY_T array<uint8_t, PUB_KEY_SIZE>
#define COMPRESSED_PUBLIC_KEY_T array<uint8_t, COMPRESSED_PUB_KEY_SIZE>
#define PRIVATE_KEY_T array<uint8_t, PRIV_KEY_SIZE>
#define SIGNATURE_T array<uint8_t, PUB_KEY_SIZE>
#define HASH vector<uint8_t>
#define UUID string

// Defining the elliptic curve to use
extern const struct uECC_Curve_t* ECC_CURVE;

// Define the cryptography namespace
namespace Crypto {
    string generateUUID();

    namespace hash {
        string sha512(string message);
        HASH sha512Vec(string message);
    }

    namespace serialize {
        string uint8ArrToString(uint8_t* arr, unsigned int len);
        vector<uint8_t> stringToUint8Array(string hex);
    }

    namespace asymmetric {
        bool verifyKeySize();

        SIGNATURE_T sign(string text, PRIVATE_KEY_T privKey);
        bool verify(string text, SIGNATURE_T signature, PUBLIC_KEY_T pubKey);

        PUBLIC_KEY_T decompressPublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey);
        struct KeyPair {
            PRIVATE_KEY_T privKey;
            PUBLIC_KEY_T pubKey;

            void setKeys(uint8_t* privKey, uint8_t* pubKey) {
                // Copy bytes from passed keys into instance properties
                copy(privKey, privKey + PRIV_KEY_SIZE, begin(this->privKey));
                copy(pubKey, pubKey + PUB_KEY_SIZE, begin(this->pubKey));
            }
        public:
            KeyPair();
            inline KeyPair(uint8_t* privKey, uint8_t* pubKey) { this->setKeys(privKey, pubKey); };

            inline PRIVATE_KEY_T getPrivate() { return this->privKey; }
            inline PUBLIC_KEY_T getPublic() { return this->pubKey; }
            COMPRESSED_PUBLIC_KEY_T getCompressedPublic();
            PUB_HASH_T getPublicHash();

            bool verifyPublicKey();
        };
    }
}


#endif //UCL_CRYPTO_HPP
