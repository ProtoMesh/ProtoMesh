#include "serialization.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

namespace ProtoMesh::cryptography::serialization {
    string uint8ArrToString(uint8_t *arr, unsigned long len) {
        stringstream ss;
        ss << hex << nouppercase << setfill('0');
        for (unsigned int i = 0; i < len; ++i)
            ss << setw(2) << static_cast<int>(arr[i]);

        return ss.str();
    }

    vector<uint8_t> stringToUint8Array(string hex) {
        vector<uint8_t> bytes;

        for (unsigned int i = 0; i < hex.length(); i += 2) {
            string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
            bytes.push_back(byte);
        }

        return bytes;
    }
#ifdef UNIT_TESTING

    SCENARIO("uint8_t <=> string conversion", "[unit_test][module][cryptography][serialization]") {
        GIVEN("An array of uint8_t's") {
            vector<uint8_t> arr = {200, 101, 230, 29, 49, 185, 102, 57, 69, 5, 9, 111};

            CAPTURE(arr);

            WHEN("it is converted to a string") {
                string str(ProtoMesh::cryptography::serialization::uint8ArrToString(&arr[0], (unsigned int) arr.size()));
                THEN("it should be valid") {
                    REQUIRE( str == "c865e61d31b966394505096f" );
                }
                AND_WHEN("that is converted back to an array") {
                    vector<uint8_t> backConverted(ProtoMesh::cryptography::serialization::stringToUint8Array(str));
                    THEN("it should match the original one") {
                        REQUIRE( backConverted == arr );
                    }
                }
            }
        }
    }

#endif
}