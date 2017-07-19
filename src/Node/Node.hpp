#ifndef LUMOS_NODE_HPP
#define LUMOS_NODE_HPP

#include <vector>
using namespace std;

#include "Network/Network.hpp"
#include "crypto/crypto.hpp"

struct NodeDeserializationError {
    enum class Kind { WrongType, InvalidData };
    Kind kind;
    std::string text;
    NodeDeserializationError(Kind kind, std::string text) : kind(kind), text(text) {}
};

class Node {
    vector<shared_ptr<Network>> networks;
public:
    Crypto::UUID uuid;
    Crypto::asym::KeyPair pair;

    Node(Crypto::UUID uuid, Crypto::asym::KeyPair keys) : uuid(uuid), pair(keys) {};

    vector<uint8_t> serializeForRegistry();

    void registerAt(shared_ptr<Network> network) {
        this->networks.push_back(network);
    };

    static Result<const lumos::network::Node*, NodeDeserializationError> deserialize(vector<uint8_t>* serializedNode) {
        using namespace lumos::network;

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(serializedNode->data(), serializedNode->size());
        if (!VerifyNodeBuffer(verifier))
            return Err(NodeDeserializationError(NodeDeserializationError::Kind::InvalidData, "Passed data is no valid node!"));

        /// Check the buffer identifier
        if (!flatbuffers::BufferHasIdentifier(serializedNode->data(), NodeIdentifier()))
            return Err(NodeDeserializationError(NodeDeserializationError::Kind::WrongType, "Passed data is not a node!"));

        /// Load and return the entry
        auto entry = GetNode(serializedNode->data());

        return Ok(entry);
    };

    static Result<Crypto::asym::PublicKey, NodeDeserializationError> getPublicKey(vector<uint8_t> serializedNode) {
        auto node(Node::deserialize(&serializedNode));
        if (node.isErr()) return Err(node.unwrapErr());

        auto key = Crypto::asym::PublicKey::fromBuffer(node.unwrap()->publicKey()->compressed());
        if (key.isErr()) return Err(NodeDeserializationError(NodeDeserializationError::Kind::InvalidData, "Public key data was invalid."));
        return Ok(key.unwrap());
    }
};


#endif //LUMOS_NODE_HPP
