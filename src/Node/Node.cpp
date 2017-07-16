//
// Created by themegatb on 7/13/17.
//

#include "Node.hpp"

vector<uint8_t> Node::serializeForRegistry() {
    using namespace lumos;
    flatbuffers::FlatBufferBuilder builder;

    /// UUID
    UUID uid(this->uuid.a, this->uuid.b, this->uuid.c, this->uuid.d);

    /// Public key
    COMPRESSED_PUBLIC_KEY_T compressedKey(this->pair.pub.getCompressed());
    auto pubKeyVec = builder.CreateVector(compressedKey.begin(), compressedKey.size());
    auto publicKey = lumos::crypto::CreatePublicKey(builder, pubKeyVec);

    network::NodeBuilder nodeBuilder(builder);
    nodeBuilder.add_uuid(&uid);
    nodeBuilder.add_publicKey(publicKey);
    auto nodeBuffer = nodeBuilder.Finish();
    builder.Finish(nodeBuffer, network::NodeIdentifier());

    /// Convert to a byte array
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> node_vec(buf, buf + builder.GetSize());
    
    return node_vec;
}
