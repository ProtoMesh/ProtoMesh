#include "crypto.hpp"

const struct uECC_Curve_t* ECC_CURVE = uECC_secp256k1();

namespace Crypto {
    namespace asym {
        bool verifyKeySize() {
            bool keySizeMatch = uECC_curve_private_key_size(ECC_CURVE) == PRIV_KEY_SIZE
                                && uECC_curve_public_key_size(ECC_CURVE) == PUB_KEY_SIZE;

            if (!keySizeMatch) cerr << "FATAL || Cipher key size mismatch! Check source code." << endl;

            return keySizeMatch;
        }

        PublicKey::PublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey) {
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_decompress(&compressedKey[0], publicKey, ECC_CURVE);
            copy(publicKey, publicKey + PUB_KEY_SIZE, begin(this->raw));
        }

        PublicKey::PublicKey(uint8_t *publicKey) {
            copy(publicKey, publicKey + PUB_KEY_SIZE, begin(this->raw));
        }

        PublicKey::PublicKey(string publicKey) {
            vector<uint8_t> compressedKey = Crypto::serialize::stringToUint8Array(publicKey);
            uint8_t key[PUB_KEY_SIZE] = {0};
            uECC_decompress(&compressedKey[0], key, ECC_CURVE);
            copy(begin(key), end(key), begin(this->raw));
        }

        COMPRESSED_PUBLIC_KEY_T PublicKey::getCompressed() {
            // Compress the public key
            uint8_t cpubKey[COMPRESSED_PUB_KEY_SIZE];
            uECC_compress(&this->raw[0], cpubKey, ECC_CURVE);

            // Copy it into an array and return it
            COMPRESSED_PUBLIC_KEY_T compressedKey;
            copy(&cpubKey[0], &cpubKey[0] + COMPRESSED_PUB_KEY_SIZE, begin(compressedKey));
            return compressedKey;
        }

        string PublicKey::getCompressedString() {
            COMPRESSED_PUBLIC_KEY_T compressedKey(this->getCompressed());
            return Crypto::serialize::uint8ArrToString(&compressedKey[0], COMPRESSED_PUB_KEY_SIZE);
        }

        PUB_HASH_T PublicKey::getHash() {
            PUB_HASH_T hash;
            string ssHash(Crypto::hash::sha512(Crypto::serialize::uint8ArrToString(&this->raw[0], PUB_KEY_SIZE)));
            copy_n(begin(ssHash), PUB_HASH_SIZE, begin(hash));
            return hash;
        }

        SIGNATURE_T sign(string text, PRIVATE_KEY_T privKey) {
            // Generate the hash and create a signature from it
            HASH hashVec = Crypto::hash::sha512Vec(text);
            uint8_t *hash = &hashVec[0];
            uint8_t sig[SIGNATURE_SIZE] = {0};
            uECC_sign(&privKey[0], hash, sizeof(hash), sig, ECC_CURVE);

            // Copy the signature into an array and return it
            SIGNATURE_T signature;
            copy(&sig[0], &sig[0] + SIGNATURE_SIZE, begin(signature));
            return signature;
        }

        bool verify(string text, SIGNATURE_T signature, PublicKey* pubKey) {
            HASH hashVec = Crypto::hash::sha512Vec(text);
            uint8_t *hash = &hashVec[0];
            return (bool) uECC_verify(&pubKey->raw[0], hash, sizeof(hash), &signature[0], ECC_CURVE);
        }
    }
}