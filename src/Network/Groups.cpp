#include "Network.hpp"

void Network::onGroupRegistryChange(RegistryEntry<vector<uint8_t>> entry) {
    // TODO Update internal state
}

Result<Crypto::UUID, GroupCreationError> Network::createGroup(Crypto::asym::KeyPair authorization, vector<Crypto::UUID> participants, Crypto::UUID groupID) {
    using namespace lumos;

    flatbuffers::FlatBufferBuilder builder;

    auto groupRegistry = this->getRegistry(GROUPS_REGISTRY);

    /// Group key
    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

    /// UUID
    lumos::UUID uid(groupID.a, groupID.b, groupID.c, groupID.d);

    /// Public key
    auto publicKey = pair.pub.toBuffer(&builder);
    auto authorizationPubKey = authorization.pub.toBuffer(&builder);

    /// Private key
    vector<uint8_t> privateKey(pair.priv.begin(), pair.priv.end());

    /// Participant creation lambda
    auto createParticipant = [&] (Crypto::UUID participantID) -> Result<flatbuffers::Offset<network::GroupParticipant>, bool> {
        lumos::UUID pid(participantID.a, participantID.b, participantID.c, participantID.d);

        /// Get the participant from the registry
        auto participatingNode = this->getRegistry(NODES_REGISTRY)->get(participantID);
        if (participatingNode.isOk()) {
            /// Extract its public key
            auto nodePubKey = Node::getPublicKey(participatingNode.unwrap());
            if (nodePubKey.isOk()) {
                /// Generate a shared key
                auto nPubKey = nodePubKey.unwrap();
                SHARED_KEY_T key = Crypto::asym::generateSharedSecret(nPubKey, authorization.priv);

                /// Generate an IV for encryption
                vector<uint8_t> iv;
                iv.insert(iv.end(), std::make_move_iterator(nPubKey.raw.begin()), std::make_move_iterator(nPubKey.raw.end()));
                iv.insert(iv.end(), pair.pub.raw.begin(), pair.pub.raw.end());
                iv = Crypto::hash::sha512Vec(iv);

                /// Encrypt the private key
                auto cipherText = Crypto::sym::encrypt(privateKey, key, iv);
                if (cipherText.isOk()) {
                    /// Create and return the GroupParticipant object
                    auto cipheredPrivateKey = builder.CreateVector(cipherText.unwrap());
                    return Ok(network::CreateGroupParticipant(builder, &pid, cipheredPrivateKey));
                } else {
                    cerr << "Error encrypting the group private key! (" << cipherText.unwrapErr().text << ")" << endl;
                }
            }
        }

        return Err(false);
    };


    /// Add all the participants
    vector<flatbuffers::Offset<network::GroupParticipant>> groupParticipants;
    for (auto participantID : participants) {
        auto participant = createParticipant(participantID);
        if (participant.isOk()) groupParticipants.push_back(participant.unwrap());
        else cerr << "Error creating participant!" << endl;
    }
    auto participantsVec = builder.CreateVector(groupParticipants);

    /// Create the group
    auto entry = network::CreateGroup(builder, &uid, publicKey, authorizationPubKey, participantsVec);
    builder.Finish(entry, network::GroupIdentifier());

    /// Serialize the group into a vector
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> entry_vec(buf, buf + builder.GetSize());

    /// Insert the group into the registry and return its UUID
    auto res = groupRegistry->set(groupID, entry_vec, authorization);
    if (res.isOk())
        return Ok(groupID);
    else
        return Err(GroupCreationError(GroupCreationError::Kind::InsertionFailed, "Couldn't insert the group into the registry. Reason: " + res.unwrapErr().text));
}