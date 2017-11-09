#ifndef PROTOMESH_ASYMMETRIC_HPP
#define PROTOMESH_ASYMMETRIC_HPP

#define CBC 1
#define EBC 0
#define AES256 1

#include <random>
#include <array>
#include <utility>
#include <vector>
#include <algorithm>
#include "flatbuffers/flatbuffers.h"
#include "result.h"
#include "uECC.h"

#include "serialization.hpp"
#include "hash.hpp"

#include "cryptography/asymmetric_generated.h"

using namespace std;

/// Defining ECC key sizes
#define PRIV_KEY_SIZE 32
#define PUB_KEY_SIZE 64
#define COMPRESSED_PUB_KEY_SIZE (PRIV_KEY_SIZE + 1)
#define SIGNATURE_SIZE PUB_KEY_SIZE

/// Define the size in bytes of the short hash to identify a public key
/// Is required to be dividable by two.
#define PUB_HASH_SIZE (PUB_KEY_SIZE / 4)
#define PUB_HASH_T array<char, PUB_HASH_SIZE>  // First PUB_HASH_SIZE characters of the HASH of the hex representation of the public key

/// Defining cryptography types
#define COMPRESSED_PUBLIC_KEY_T array<uint8_t, COMPRESSED_PUB_KEY_SIZE>
#define PRIVATE_KEY_T array<uint8_t, PRIV_KEY_SIZE>
#define SIGNATURE_T array<uint8_t, PUB_KEY_SIZE>
#define SHARED_KEY_T vector<uint8_t>

/// Defining the elliptic curve to use
extern const struct uECC_Curve_t* ECC_CURVE;


namespace ProtoMesh::cryptography::asymmetric {
    bool verifyKeySize();


    struct PublicKeyDeserializationError {
        enum class Kind { KeySizeMismatch };
        Kind kind;
        std::string text;
        PublicKeyDeserializationError(Kind kind, std::string text) : kind(kind), text(std::move(text)) {}
    };

    struct PublicKey {
    public:
        array<uint8_t, PUB_KEY_SIZE> raw;

        static Result<PublicKey, PublicKeyDeserializationError> fromBuffer(const flatbuffers::Vector<uint8_t>* buffer);

        explicit PublicKey(COMPRESSED_PUBLIC_KEY_T compressedKey);
        explicit PublicKey(uint8_t* publicKey);
        explicit PublicKey(string publicKey); // takes a hex representation of a compressed pub key

        string getCompressedString() const;

        COMPRESSED_PUBLIC_KEY_T getCompressed() const;

        PUB_HASH_T getHash() const;

        flatbuffers::Offset<ProtoMesh::scheme::cryptography::PublicKey>
        toBuffer(flatbuffers::FlatBufferBuilder *builder) const;
    };

    struct KeyPair {
    public:
        PRIVATE_KEY_T priv;
        PublicKey pub;
        inline KeyPair(uint8_t* privKey, uint8_t* pubKey) : pub(pubKey) {
            copy(privKey, privKey + PRIV_KEY_SIZE, begin(this->priv));
        };
    };

    // TODO --important-- provide a platform unrelated PRNG
    inline KeyPair generateKeyPair() {
        // Generate two keys
        uint8_t privateKey[PRIV_KEY_SIZE] = {0};
        uint8_t publicKey[PUB_KEY_SIZE] = {0};
        uECC_make_key(publicKey, privateKey, ECC_CURVE);

        return {std::move(privateKey), std::move(publicKey)};
    }

    SIGNATURE_T sign(vector<uint8_t> text, PRIVATE_KEY_T privKey);
    bool verify(vector<uint8_t> text, SIGNATURE_T signature, PublicKey* pubKey);

    SHARED_KEY_T generateSharedSecret(PublicKey publicKey, PRIVATE_KEY_T privateKey);
}


#endif //PROTOMESH_ASYMMETRIC_HPP
