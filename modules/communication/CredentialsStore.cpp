#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "CredentialsStore.hpp"

namespace ProtoMesh::communication {

    Result<void, CredentialsStore::CredentialsError>
    CredentialsStore::insertKey(cryptography::UUID deviceID, cryptography::asymmetric::PublicKey key) {
        auto existingKeyResult = this->getKey(deviceID);

        if (existingKeyResult.isOk()) {
            cryptography::asymmetric::PublicKey existingKey = existingKeyResult.unwrap();

            if (existingKey == key) return Ok();
            else return Err(CredentialsError::MismatchingKeyExists); // TODO Print a warning
        }

        this->knownHosts.insert({deviceID, key});

        return Ok();
    }

    Result<cryptography::asymmetric::PublicKey, CredentialsStore::CredentialsError>
    CredentialsStore::getKey(cryptography::UUID deviceID) {
        auto key = this->knownHosts.find(deviceID);

        if (key != this->knownHosts.end())
            return Ok(key->second);

        return Err(CredentialsError::KeyNotFound);
    }

#ifdef UNIT_TESTING

    SCENARIO("Storing and retrieving credentials", "[unit_test][module][communication]") {
        GIVEN("two public -> uuid correlations and a CredentialsStore instance") {
            cryptography::asymmetric::PublicKey key1 = cryptography::asymmetric::generateKeyPair().pub;
            cryptography::asymmetric::PublicKey key2 = cryptography::asymmetric::generateKeyPair().pub;

            cryptography::UUID id1;
            cryptography::UUID id2;

            CredentialsStore credentials;

            WHEN("both correlations are inserted into the store") {
                auto result1 = credentials.insertKey(id1, key1);
                auto result2 = credentials.insertKey(id2, key2);

                THEN("both insertions should be successful") {
                    REQUIRE(result1.isOk());
                    REQUIRE(result2.isOk());

                    AND_THEN("both keys should be retrievable") {
                        auto retrievedKey1 = credentials.getKey(id1);
                        auto retrievedKey2 = credentials.getKey(id2);

                        REQUIRE(retrievedKey1.unwrap() == key1);
                        REQUIRE(retrievedKey2.unwrap() == key2);
                    }
                }
            }
        }
    }

#endif
}