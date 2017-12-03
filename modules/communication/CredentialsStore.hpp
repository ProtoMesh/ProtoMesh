#ifndef PROTOMESH_CREDENTIALSSTORE_HPP
#define PROTOMESH_CREDENTIALSSTORE_HPP

#include <unordered_map>
#include <asymmetric.hpp>
#include <uuid.hpp>

#include "result.h"

namespace ProtoMesh::communication {

    class CredentialsStore {
        // TODO Possibly store/cache shared secrets
        unordered_map<cryptography::UUID, cryptography::asymmetric::PublicKey> knownHosts;

    public:
        enum class CredentialsError {
            KeyNotFound,
            MismatchingKeyExists
        };

        Result<cryptography::asymmetric::PublicKey, CredentialsError> getKey(cryptography::UUID deviceID);

        Result<void, CredentialsError> insertKey(cryptography::UUID deviceID, cryptography::asymmetric::PublicKey key);
    };

}


#endif //PROTOMESH_CREDENTIALSSTORE_HPP
