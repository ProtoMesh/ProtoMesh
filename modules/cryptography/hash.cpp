#include "hash.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

namespace ProtoMesh::cryptography::hash {
    string sha512(vector<uint8_t> message) {
        string text(message.begin(), message.end());
        return sw::sha512::calculate(text);
    }

    HASH sha512Vec(vector<uint8_t> message) {
        string hash = sha512(message);
        vector<uint8_t> sha512Vector(hash.begin(), hash.end());
        return sha512Vector;
    }

#ifdef UNIT_TESTING

    SCENARIO("SHA512 creation", "[unit_test][module][cryptography][hash][sha512]") {
        GIVEN("An SHA512 hash of a message") {
            vector<uint8_t> msg = {115, 111, 109, 101, 83, 116, 114, 105, 110, 103};
            string validHash("bc911c34051d0523314a9c121d06d4907fc4a91ed73312c9a87d6f5ac969095de7a7c88d43cea88ced5888df1d0d01d8a2f13a0313fed05362626260e009dd51");
            string hash(sha512(msg));

            CAPTURE(msg);

            THEN("it should be valid") {
                REQUIRE( hash == validHash );
            }

            WHEN("it is converted to a vector") {
                HASH vec(sha512Vec(msg));
                THEN("it should match its string representation") {
                    std::string convertedToString;
                    for (uint8_t i : vec) convertedToString += (char) i;
                    REQUIRE( convertedToString == validHash );
                }
            }
        }
    }

#endif
}