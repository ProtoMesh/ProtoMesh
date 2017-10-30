#include "crypto.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

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
            uECC_decompress(compressedKey.data(), publicKey, ECC_CURVE);
            copy(publicKey, publicKey + PUB_KEY_SIZE, begin(this->raw));
        }

        PublicKey::PublicKey(uint8_t *publicKey) {
            copy(publicKey, publicKey + PUB_KEY_SIZE, begin(this->raw));
        }

        PublicKey::PublicKey(string publicKey) {
            vector<uint8_t> compressedKey = Crypto::serialize::stringToUint8Array(publicKey);
            uint8_t key[PUB_KEY_SIZE] = {0};
            uECC_decompress(compressedKey.data(), key, ECC_CURVE);
            copy(begin(key), end(key), begin(this->raw));
        }

        Result<PublicKey, PublicKeyDeserializationError> PublicKey::fromBuffer(const flatbuffers::Vector<uint8_t>* buffer) {
            vector<uint8_t> compressedData(buffer->begin(), buffer->end());
            if (compressedData.size() != COMPRESSED_PUB_KEY_SIZE) return Err(PublicKeyDeserializationError(PublicKeyDeserializationError::Kind::KeySizeMismatch, "Compressed key size mismatch."));
            array<uint8_t, COMPRESSED_PUB_KEY_SIZE> compressedKey;
            copy_n(compressedData.begin(), COMPRESSED_PUB_KEY_SIZE, compressedKey.begin());

            return Ok(Crypto::asym::PublicKey(compressedKey));
        }

        COMPRESSED_PUBLIC_KEY_T PublicKey::getCompressed() {
            // Compress the public key
            uint8_t cpubKey[COMPRESSED_PUB_KEY_SIZE];
            uECC_compress(this->raw.data(), cpubKey, ECC_CURVE);

            // Copy it into an array and return it
            COMPRESSED_PUBLIC_KEY_T compressedKey;
            copy(&cpubKey[0], &cpubKey[0] + COMPRESSED_PUB_KEY_SIZE, begin(compressedKey));
            return compressedKey;
        }

        string PublicKey::getCompressedString() {
            COMPRESSED_PUBLIC_KEY_T compressedKey(this->getCompressed());
            return Crypto::serialize::uint8ArrToString(compressedKey.data(), COMPRESSED_PUB_KEY_SIZE);
        }

        PUB_HASH_T PublicKey::getHash() {
            PUB_HASH_T hash;
            vector<uint8_t> data(this->raw.begin(), this->raw.end());
            string ssHash(Crypto::hash::sha512(data));
            copy_n(begin(ssHash), PUB_HASH_SIZE, begin(hash));
            return hash;
        }

        flatbuffers::Offset<protoMesh::crypto::PublicKey> PublicKey::toBuffer(flatbuffers::FlatBufferBuilder* builder) {
            COMPRESSED_PUBLIC_KEY_T compressedKey(this->getCompressed());
            auto pubKeyVec = builder->CreateVector(compressedKey.begin(), compressedKey.size());
            return protoMesh::crypto::CreatePublicKey(*builder, pubKeyVec);
        }

        SIGNATURE_T sign(vector<uint8_t> text, PRIVATE_KEY_T privKey) {
            // Generate the hash and create a signature from it
            HASH hashVec = Crypto::hash::sha512Vec(text);
            uint8_t *hash = hashVec.data();
            uint8_t sig[SIGNATURE_SIZE] = {0};
            uECC_sign(privKey.data(), hash, sizeof(hash), sig, ECC_CURVE);

            // Copy the signature into an array and return it
            SIGNATURE_T signature;
            copy(&sig[0], &sig[0] + SIGNATURE_SIZE, begin(signature));
            return signature;
        }

        bool verify(vector<uint8_t> text, SIGNATURE_T signature, PublicKey* pubKey) {
            HASH hashVec = Crypto::hash::sha512Vec(text);
            uint8_t *hash = hashVec.data();
            return (bool) uECC_verify(pubKey->raw.data(), hash, sizeof(hash), signature.data(), ECC_CURVE);
        }

        SHARED_KEY_T generateSharedSecret(PublicKey publicKey, PRIVATE_KEY_T privateKey) {
            uint8_t sharedSecret[32] = {0};
            uECC_shared_secret(publicKey.raw.data(), privateKey.data(), sharedSecret, ECC_CURVE);
            /// Copy the secret into a vector
            vector<uint8_t> secret(sharedSecret, &sharedSecret[0] + 32);

            /// Hash the key and return it
            return Crypto::hash::sha512Vec(secret);
        }

#ifdef UNIT_TESTING
        SCENARIO("Elliptic curve cryptography", "[crypto][asym]") {
            THEN("the key sizes should be valid") {
                REQUIRE(verifyKeySize());
            }

            GIVEN("a generated KeyPair") {
                KeyPair pair(Crypto::asym::generateKeyPair());

                GIVEN("a second KeyPair") {
                    KeyPair pair2(Crypto::asym::generateKeyPair());

                    WHEN("the shared secret of both (in both directions) is generated") {
                        SHARED_KEY_T key1(Crypto::asym::generateSharedSecret(pair.pub, pair2.priv));
                        SHARED_KEY_T key2(Crypto::asym::generateSharedSecret(pair2.pub, pair.priv));

                        THEN("the two keys should be identical") {
                            REQUIRE( key1 == key2 );
                        }
                    }
                }

                GIVEN("the public key of this KeyPair") {
                    PublicKey pub = pair.pub;

                    WHEN("it is compressed into a string") {
                        string compressedPub(pub.getCompressedString());

                        AND_WHEN("it is converted back into an object") {
                            PublicKey backConverted(compressedPub);

                            THEN("its raw data should match the original key") {
                                REQUIRE( backConverted.raw == pub.raw );
                            }
                        }
                    }

                    WHEN("it is compressed into an array") {
                        COMPRESSED_PUBLIC_KEY_T compressedPub(pub.getCompressed());

                        AND_WHEN("it is converted back into an object") {
                            PublicKey backConverted(compressedPub);

                            THEN("its raw data should match the original key") {
                                REQUIRE( backConverted.raw == pub.raw );
                            }
                        }
                    }
                }

                GIVEN("the signature of a message") {
                    vector<uint8_t> msg = {1,2,3,4,5};
                    SIGNATURE_T sig(sign(msg, pair.priv));

                    THEN("it should be valid when validated w/ the corresponding public key") {
                        REQUIRE(verify(msg, sig, &pair.pub));
                    }

                    THEN("it should not be valid when validated w/ a different public key") {
                        PublicKey otherPub = generateKeyPair().pub;
                        REQUIRE_FALSE(verify(msg, sig, &otherPub));
                    }

                    THEN("it should not be valid when validated w/ different text") {
                        msg.push_back(6);
                        REQUIRE_FALSE(verify(msg, sig, &pair.pub));
                    }

                    THEN("it should not be valid when the signature is altered") {
                        SIGNATURE_T alteredSig(sig);
                        alteredSig[0] /= 2;
                        REQUIRE_FALSE(verify(msg, alteredSig, &pair.pub));
                    }
                }
            }
        }

#endif
    }
}