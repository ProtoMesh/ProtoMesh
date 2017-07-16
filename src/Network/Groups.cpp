#include "Network.hpp"

void Network::onGroupRegistryChange(RegistryEntry<vector<uint8_t>> entry) {
    // TODO Update internal state
}

Result<Crypto::UUID, GroupCreationError> Network::createGroup(Crypto::asym::KeyPair authorization) {
    using namespace lumos::network;

    flatbuffers::FlatBufferBuilder builder;

    /// Group key
    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

    /// UUID
    Crypto::UUID uuid;
    lumos::UUID uid(uuid.a, uuid.b, uuid.c, uuid.d);

    /// Public key
    COMPRESSED_PUBLIC_KEY_T compressedKey(pair.pub.getCompressed());
    auto pubKeyVec = builder.CreateVector(compressedKey.begin(), compressedKey.size());
    auto publicKey = lumos::crypto::CreatePublicKey(builder, pubKeyVec);

    /// Participants
    auto createParticipant = [&] (Crypto::UUID participantID) {
        lumos::UUID pid(participantID.a, participantID.b, participantID.c, participantID.d);

        /// Get the participants public key from the registry
        auto participatingNode = this->getRegistry(GROUPS_REGISTRY)->get(participantID);

        auto participant = CreateGroupParticipant(builder, &pid, builder.CreateVector({}));
    };

//    auto entry = CreateGroup(builder, &uid, publicKey, );
//    builder.Finish(entry, GroupIdentifier());
//
//    uint8_t *buf = builder.GetBufferPointer();
//    vector<uint8_t> entry_vec(buf, buf + builder.GetSize());

//    return make_tuple(GroupCreationResult::AlreadyPresent, Crypto::UUID::Empty());
    return Err(GroupCreationError(GroupCreationError::Kind::AlreadyPresent, "Actually it could be present the code is just not there yet!"));
}