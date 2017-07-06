#include "crypto.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

namespace Crypto {
    string generateUUID() {
        random_device rd;
        default_random_engine generator(rd());
        uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

        stringstream ss;
        ss << hex << nouppercase << setfill('0');

        uint32_t a = distribution(generator);
        uint32_t b = distribution(generator);
        uint32_t c = distribution(generator);
        uint32_t d = distribution(generator);

        ss << setw(8) << (a) << '-';
        ss << setw(4) << (b >> 16) << '-';
        ss << setw(4) << (b & 0xFFFF) << '-';
        ss << setw(4) << (c >> 16) << '-';
        ss << setw(4) << (c & 0xFFFF);
        ss << setw(8) << d;

        return ss.str();
    }

#ifdef UNIT_TESTING
    SCENARIO("UUID Creation", "[crypto][uuid]") {
        GIVEN("A uuid version 4") {
            string uuid(Crypto::generateUUID());

            CAPTURE(uuid);

            THEN("it should be 36 characters long") {
                REQUIRE( uuid.size() == 36 );
            }
            AND_THEN("it should consist of 5 blocks") {
                int i = 0;
                for (char c : uuid) if (c == '-') i++;
                REQUIRE( i == 4 );
            }
        }
    }

#endif

    namespace hash {
        string sha512(string message) { return sw::sha512::calculate(message); }
        HASH sha512Vec(string message) {
            string sha512 = Crypto::hash::sha512(message);
            vector<uint8_t> sha512Vector(sha512.begin(), sha512.end());
            return sha512Vector;
        }

#ifdef UNIT_TESTING
        SCENARIO("SHA512 creation", "[crypto][hash][sha512]") {
            GIVEN("An SHA512 hash of a message") {
                string msg("someString");
                string validHash("bc911c34051d0523314a9c121d06d4907fc4a91ed73312c9a87d6f5ac969095de7a7c88d43cea88ced5888df1d0d01d8a2f13a0313fed05362626260e009dd51");
                string hash(Crypto::hash::sha512(msg));

                CAPTURE(msg);

                THEN("it should be valid") {
                    REQUIRE( hash == validHash );
                }

                WHEN("it is converted to a vector") {
                    HASH vec(Crypto::hash::sha512Vec(msg));
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

    namespace serialize {
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
        SCENARIO("uint8_t <=> string conversion", "[crypto][serialize]") {
            GIVEN("An array of uint8_t's") {
                vector<uint8_t> arr = {200, 101, 230, 29, 49, 185, 102, 57, 69, 5, 9, 111};

                CAPTURE(arr);

                WHEN("it is converted to a string") {
                    string str(Crypto::serialize::uint8ArrToString(&arr[0], (unsigned int) arr.size()));
                    THEN("it should be valid") {
                        REQUIRE( str == "c865e61d31b966394505096f" );
                    }
                    AND_WHEN("that is converted back to an array") {
                        vector<uint8_t> backConverted(Crypto::serialize::stringToUint8Array(str));
                        THEN("it should match the original one") {
                            REQUIRE( backConverted == arr );
                        }
                    }
                }
            }
        }

#endif
    }
}