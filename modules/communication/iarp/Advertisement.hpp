#ifndef PROTOMESH_ADVERTISEMENT_HPP
#define PROTOMESH_ADVERTISEMENT_HPP

#include <utility>

#include "transmission.hpp"
#include "uuid.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/iarp/advertisement_generated.h"

namespace ProtoMesh::communication::Routing::IARP {
    class Advertisement {
    public:
        cryptography::UUID uuid;
        cryptography::asymmetric::PublicKey pubKey;

        vector<cryptography::UUID> route;
        unsigned int interval;

        enum class AdvertisementDeserializationError {
            INVALID_IDENTIFIER,
            INVALID_BUFFER,
            INVALID_PUB_KEY
        };

        explicit Advertisement(cryptography::UUID uuid,
                               cryptography::asymmetric::PublicKey pubKey,
                               vector<cryptography::UUID> route = {},
                               unsigned int interval = 10000)
                : uuid(uuid), pubKey(pubKey), route(std::move(route)), interval(interval) {};

        void addHop(cryptography::UUID uuid);

        vector<uint8_t> serialize() const;

        static Advertisement build(cryptography::UUID uuid, cryptography::asymmetric::KeyPair key) {
            return Advertisement(uuid, key.pub);
        }

        static Result<Advertisement, AdvertisementDeserializationError> fromBuffer(vector<uint8_t> buffer);
    };
}


#endif //PROTOMESH_ADVERTISEMENT_HPP
