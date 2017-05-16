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
const struct uECC_Curve_t* ECC_CURVE = uECC_secp256k1();

// Define the cryptography namespace
namespace Crypto {
    string calculateSHA512(string message) {
        return sw::sha512::calculate(message);
    }

    HASH calculateSHA512Vector(string message) {
        string sha512 = calculateSHA512(message);
        vector<uint8_t> sha512Vector(sha512.begin(), sha512.end());

        return sha512Vector;
    }

    string generateUUID() {
        random_device rd;
        default_random_engine generator(rd());
        uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

        stringstream ss;
        ss << hex << nouppercase << setfill('0');

        uint32_t a = distribution(generator);
        uint32_t b = distribution(generator);
        uint32_t c = distribution(generator);
        uint32_t d = distribution(generator);

        ss << setw(8) << (a) << '-';
        ss << setw(4) << (b >> 16) << '-';
        ss << setw(4) << (b & 0xFFFF) << '-';
        ss << setw(4) << (c >> 16 ) << '-';
        ss << setw(4) << (c & 0xFFFF);
        ss << setw(8) << d;

        return ss.str();
    }

    string serializeIntArray(uint8_t* arr, unsigned int len) {
        stringstream ss;
        ss << hex << nouppercase << setfill('0');
        for (int i = 0; i < len; ++i)
            ss << setw(2) << static_cast<int>(arr[i]);

        return ss.str();
    }

    vector<uint8_t> deserializeHexString(string hex) {
        vector<uint8_t> bytes;

        for (unsigned int i = 0; i < hex.length(); i += 2) {
            string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
            bytes.push_back(byte);
        }

        return bytes;
    }

    namespace asymmetric {
        bool verifyKeySize() {
            bool keySizeMatch = uECC_curve_private_key_size(ECC_CURVE) == PRIV_KEY_SIZE
                                && uECC_curve_public_key_size(ECC_CURVE) == PUB_KEY_SIZE;

            if (!keySizeMatch) cerr << "FATAL || Cipher key size mismatch! Check source code." << endl;

            return keySizeMatch;
        }

        PUBLIC_KEY_T decompressPublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey) {
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_decompress(&compressedKey[0], publicKey, ECC_CURVE);

            PUBLIC_KEY_T pubKey;
            copy(publicKey, publicKey + PUB_KEY_SIZE, begin(pubKey));
            return pubKey;
        }

        struct KeyPair {
            PRIVATE_KEY_T privKey;
            PUBLIC_KEY_T pubKey;

            void setKeys(uint8_t* privKey, uint8_t* pubKey) {
                // Copy bytes from passed keys into instance properties
                copy(privKey, privKey + PRIV_KEY_SIZE, begin(this->privKey));
                copy(pubKey, pubKey + PUB_KEY_SIZE, begin(this->pubKey));
            }
        public:
            KeyPair(uint8_t* privKey, uint8_t* pubKey) { this->setKeys(privKey, pubKey); };
            KeyPair() {
                // Generate two keys
                uint8_t privateKey[PRIV_KEY_SIZE] = {0};
                uint8_t publicKey[PUB_KEY_SIZE] = {0};
                uECC_make_key(publicKey, privateKey, ECC_CURVE);

                // Set the keys
                this->setKeys(privateKey, publicKey);
            }

            bool verifyPublicKey() {
                // Calculate public key
                uint8_t publicKey[PUB_KEY_SIZE] = {0};
                uECC_compute_public_key(&this->privKey[0], publicKey, ECC_CURVE);

                // Compare keys
                for (int i = 0; i < PUB_KEY_SIZE; ++i)
                    if (publicKey[i] != this->pubKey[i]) return false;
                return true;
            }

            PRIVATE_KEY_T getPrivate() { return this->privKey; }
            PUBLIC_KEY_T getPublic() { return this->pubKey; }
            COMPRESSED_PUBLIC_KEY_T getCompressedPublic() {
                // Compress the public key
                uint8_t cpubKey[COMPRESSED_PUB_KEY_SIZE];
                uECC_compress(&this->pubKey[0], cpubKey, ECC_CURVE);

                // Copy it into an array and return it
                COMPRESSED_PUBLIC_KEY_T compressedKey;
                copy(&cpubKey[0], &cpubKey[0] + COMPRESSED_PUB_KEY_SIZE, begin(compressedKey));
                return compressedKey;
            }

            PUB_HASH_T getPublicHash() {
                PUB_HASH_T hash;
                string ssHash(Crypto::calculateSHA512(Crypto::serializeIntArray(&this->pubKey[0], PUB_KEY_SIZE)));
                copy_n(begin(ssHash), PUB_HASH_SIZE, begin(hash));
                return hash;
            }
        };

        SIGNATURE_T sign(string text, PRIVATE_KEY_T privKey) {
            // Generate the hash and create a signature from it
            uint8_t *hash = &Crypto::calculateSHA512Vector(text)[0];
            uint8_t sig[SIGNATURE_SIZE] = {0};
            uECC_sign(&privKey[0], hash, sizeof(hash), sig, ECC_CURVE);

            // Copy the signature into an array and return it
            SIGNATURE_T signature;
            copy(&sig[0], &sig[0] + SIGNATURE_SIZE, begin(signature));
            return signature;
        }

        bool verify(string text, SIGNATURE_T signature, PUBLIC_KEY_T pubKey) {
            uint8_t *hash = &Crypto::calculateSHA512Vector(text)[0];
            return (bool) uECC_verify(&pubKey[0], hash, sizeof(hash), &signature[0], ECC_CURVE);
        }
    }
}


#endif //UCL_CRYPTO_HPP
