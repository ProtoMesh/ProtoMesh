#include "crypto.hpp"

const struct uECC_Curve_t* ECC_CURVE = uECC_secp256k1();

namespace Crypto {
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

        SIGNATURE_T sign(string text, PRIVATE_KEY_T privKey) {
            // Generate the hash and create a signature from it
            uint8_t *hash = &Crypto::hash::sha512Vec(text)[0];
            uint8_t sig[SIGNATURE_SIZE] = {0};
            uECC_sign(&privKey[0], hash, sizeof(hash), sig, ECC_CURVE);

            // Copy the signature into an array and return it
            SIGNATURE_T signature;
            copy(&sig[0], &sig[0] + SIGNATURE_SIZE, begin(signature));
            return signature;
        }

        bool verify(string text, SIGNATURE_T signature, PUBLIC_KEY_T pubKey) {
            uint8_t *hash = &Crypto::hash::sha512Vec(text)[0];
            return (bool) uECC_verify(&pubKey[0], hash, sizeof(hash), &signature[0], ECC_CURVE);
        }

        KeyPair::KeyPair() {
            // Generate two keys
            uint8_t privateKey[PRIV_KEY_SIZE] = {0};
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_make_key(publicKey, privateKey, ECC_CURVE);

            // Set the keys
            this->setKeys(privateKey, publicKey);
        }

        bool KeyPair::verifyPublicKey() {
            // Calculate public key
            uint8_t publicKey[PUB_KEY_SIZE] = {0};
            uECC_compute_public_key(&this->privKey[0], publicKey, ECC_CURVE);

            // Compare keys
            for (int i = 0; i < PUB_KEY_SIZE; ++i)
                if (publicKey[i] != this->pubKey[i]) return false;
            return true;
        }

        COMPRESSED_PUBLIC_KEY_T KeyPair::getCompressedPublic() {
            // Compress the public key
            uint8_t cpubKey[COMPRESSED_PUB_KEY_SIZE];
            uECC_compress(&this->pubKey[0], cpubKey, ECC_CURVE);

            // Copy it into an array and return it
            COMPRESSED_PUBLIC_KEY_T compressedKey;
            copy(&cpubKey[0], &cpubKey[0] + COMPRESSED_PUB_KEY_SIZE, begin(compressedKey));
            return compressedKey;
        }

        PUB_HASH_T KeyPair::getPublicHash() {
            PUB_HASH_T hash;
            string ssHash(Crypto::hash::sha512(Crypto::serialize::uint8ArrToString(&this->pubKey[0], PUB_KEY_SIZE)));
            copy_n(begin(ssHash), PUB_HASH_SIZE, begin(hash));
            return hash;
        }
    }
}