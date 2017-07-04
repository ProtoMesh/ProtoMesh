#ifndef OPEN_HOME_CRYPTO_HPP
#define OPEN_HOME_CRYPTO_HPP

#include <random>
#include <array>
#include <vector>
#include <algorithm>

#include "sha512.hpp"
#include "uECC.h"
#include "../api/network.hpp"

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
#define NETWORK_KEY_T COMPRESSED_PUBLIC_KEY_T
#define NETWORK_KEY_SIZE COMPRESSED_PUB_KEY_SIZE
#define SIGNATURE_T array<uint8_t, PUB_KEY_SIZE>
#define HASH vector<uint8_t>
//#define UUID string

// Defining the elliptic curve to use
extern const struct uECC_Curve_t* ECC_CURVE;

// Define the cryptography namespace
namespace Crypto {
    class UUID {
    public:
        ulong a, b, c, d;

        static UUID Empty() {
            return UUID(0, 0, 0, 0);
        }
        UUID(ulong a, ulong b, ulong c, ulong d) : a(a), b(b), c(c), d(d) {};
        UUID() {
            random_device rd;
            default_random_engine generator(rd());
            uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

            a = distribution(generator);
            b = distribution(generator);
            c = distribution(generator);
            d = distribution(generator);
        }

        operator string() const {
            stringstream ss;
            ss << hex << nouppercase << setfill('0');

            ss << setw(8) << (a) << '-';
            ss << setw(4) << (b >> 16) << '-';
            ss << setw(4) << (b & 0xFFFF) << '-';
            ss << setw(4) << (c >> 16) << '-';
            ss << setw(4) << (c & 0xFFFF);
            ss << setw(8) << d;

            return ss.str();
        }
        inline bool operator==(const UUID &other) {
            return a == other.a && b == other.b && c == other.c && d == other.d;
        }
        inline bool operator!=(const UUID &other) {
            return !(&other == this);
        }
        inline bool operator>(const UUID &other) {
            return a >= other.a && b >= other.b && c >= other.c && d > other.c;
        }
        inline bool operator<(const UUID &other) {
            return &other >= this;
        }
    };

    string generateUUID();

    namespace hash {
        string sha512(string message);
        HASH sha512Vec(string message);
    }

    namespace serialize {
        string uint8ArrToString(uint8_t* arr, unsigned int len);
        vector<uint8_t> stringToUint8Array(string hex);
    }

//    namespace sym {
//        vector<uint8_t> encrypt(string text, string key);
//        string decrypt(vector<uint8_t> ciphertext, string key);
//    }

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

    namespace net {
        class EncryptedBroadcastSocket : public BroadcastSocket {
            BCAST_SOCKET_T sock;
            NETWORK_KEY_T key;
        public:
            inline EncryptedBroadcastSocket(BCAST_SOCKET_T sock, NETWORK_KEY_T key) : sock(sock), key(key) {};
            inline void broadcast(vector<uint8_t> message) override {
                // TODO Encrypt message
                this->sock->broadcast(message);
            };
            inline void send(string ip, unsigned short port, string message) override {
                // TODO Encrypt message
                this->sock->send(ip, port, message);
            };
            inline int recv(vector<uint8_t> *msg, unsigned int timeout_ms) override {
                // TODO Decrypt message
                return this->sock->recv(msg, timeout_ms);
            };
        };

        class EncryptedNetworkProvider : public NetworkProvider {
            shared_ptr<NetworkProvider> net;
            NETWORK_KEY_T key;
        public:
            inline EncryptedNetworkProvider(NetworkProvider* net, NETWORK_KEY_T key) : net(net), key(key) {};
            BCAST_SOCKET_T createBroadcastSocket(std::string multicastGroup, unsigned short port) override {
                return BCAST_SOCKET_T(
                        new EncryptedBroadcastSocket(this->net->createBroadcastSocket(multicastGroup, port), this->key)
                );
            };
            SOCKET_T openChannel(string ip) override { return nullptr; };
        };
    }
}


#endif //OPEN_HOME_CRYPTO_HPP
