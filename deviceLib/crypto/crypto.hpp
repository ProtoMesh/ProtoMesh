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

    namespace asym {
        bool verifyKeySize();

        struct PublicKey {
        public:
            array<uint8_t, PUB_KEY_SIZE> raw;

            PublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey);
            PublicKey(uint8_t* publicKey);
            PublicKey(string publicKey); // takes a hex representation of a compressed pub key

            string getCompressedString();
            COMPRESSED_PUBLIC_KEY_T getCompressed();
            PUB_HASH_T getHash();
        };

        struct KeyPair {
        public:
            PRIVATE_KEY_T priv;
            PublicKey pub;
            inline KeyPair(uint8_t* privKey, uint8_t* pubKey) : pub(pubKey) {
                copy(privKey, privKey + PRIV_KEY_SIZE, begin(this->priv));
            };
        };

        inline KeyPair generateKeyPair() {
            // Generate two keys
            uint8_t privateKey[PRIV_KEY_SIZE] = {0};
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_make_key(publicKey, privateKey, ECC_CURVE);

            return KeyPair(privateKey, publicKey);
        }

        SIGNATURE_T sign(string text, PRIVATE_KEY_T privKey);
        bool verify(string text, SIGNATURE_T signature, PublicKey* pubKey);
    }
}


#endif //UCL_CRYPTO_HPP
