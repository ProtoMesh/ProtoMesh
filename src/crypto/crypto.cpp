#include "crypto.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

namespace Crypto {
    void UUID::generateRandom() {
        random_device rd;
        default_random_engine generator(rd());
        uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

        a = distribution(generator);
        b = distribution(generator);
        c = distribution(generator);
        d = distribution(generator);
    }

    UUID::UUID()  {
        this->generateRandom();
    }

    UUID::UUID(UUIDType type) {
        this->generateRandom();
        this->type = type;
    }

    UUID::UUID(const hoMesh::UUID *id) {
        a = (uint32_t) id->a();
        b = (uint32_t) id->b();
        c = (uint32_t) id->c();
        d = (uint32_t) id->d();
        type = (UUIDType) id->type();
    }

    vector<uint8_t> UUID::toVector() const {
        vector<uint8_t> vec;

        vec.push_back((uint8_t) this->type);

        for (uint32_t v : {this->a, this->b, this->c, this->d}) {
            vec.push_back((uint8_t) (v & 0x000000ff));
            vec.push_back((uint8_t) (v & 0x0000ff00) >> 8);
            vec.push_back((uint8_t) (v & 0x00ff0000) >> 16);
            vec.push_back((uint8_t) (v & 0xff000000) >> 24);
        }

        return vec;
    }

    UUID::operator string() const {
        stringstream ss;
        ss << hex << nouppercase << setfill('0');

        ss << (uint8_t) type << '-';
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
        GIVEN("Two UUIDs of the same type") {
            UUID uuid1;
            UUID uuid2;

            THEN("they may not be equal") {
                REQUIRE(uuid1 != uuid2);
                REQUIRE_FALSE(uuid1 == uuid2);
            }

            THEN("their string representations may not be equal") {
                REQUIRE(string(uuid1) != string(uuid2));
            }
        }

        GIVEN("Two equal UUIDs with differing types") {
            UUID uuid1(UUIDType::Device);
            UUID uuid2(uuid1.a, uuid1.b, uuid1.c, uuid1.d, UUIDType::Endpoint);

            THEN("they may not be equal") {
                REQUIRE(uuid1 != uuid2);
                REQUIRE_FALSE(uuid1 == uuid2);
            }

            THEN("their string representations may not be equal") {
                REQUIRE(string(uuid1) != string(uuid2));
            }
        }
    }
#endif

    namespace hash {
        string sha512(vector<uint8_t> message) {
            string text(message.begin(), message.end());
            return sw::sha512::calculate(text);
        }
        HASH sha512Vec(vector<uint8_t> message) {
            string sha512 = Crypto::hash::sha512(message);
            vector<uint8_t> sha512Vector(sha512.begin(), sha512.end());
            return sha512Vector;
        }

#ifdef UNIT_TESTING
        SCENARIO("SHA512 creation", "[crypto][hash][sha512]") {
            GIVEN("An SHA512 hash of a message") {
                vector<uint8_t> msg = {115, 111, 109, 101, 83, 116, 114, 105, 110, 103};
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