#include "uuid.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

namespace ProtoMesh::cryptography {

    void UUID::generateRandom() {
        random_device rd;
        default_random_engine generator(rd());
        uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

        a = distribution(generator);
        b = distribution(generator);
        c = distribution(generator);
        d = distribution(generator);
    }

    UUID::UUID() {
        this->generateRandom();
    }

    UUID::UUID(const scheme::cryptography::UUID *id) {
        a = (uint32_t) id->a();
        b = (uint32_t) id->b();
        c = (uint32_t) id->c();
        d = (uint32_t) id->d();
    }

    scheme::cryptography::UUID UUID::toScheme() const {
        return {a, b, c, d};
    }

    UUID::operator string() const {
        stringstream ss;
        ss << hex << nouppercase << setfill('0');

        ss << setw(8) << (a) << '-';
        ss << setw(4) << (b >> 16) << '-';
        ss << setw(4) << (b & 0xFFFF) << '-';
        ss << setw(4) << (c >> 16) << '-';
        ss << setw(4) << (c & 0xFFFF);
        ss << setw(8) << d;

        return ss.str();
    }

#ifdef UNIT_TESTING
    SCENARIO("UUID Creation", "[module][cryptography][uuid]") {
        GIVEN("Two distinct UUIDs") {
            UUID uuid1;
            UUID uuid2;

            THEN("they may not be equal") {
                REQUIRE(uuid1 != uuid2);
                REQUIRE_FALSE(uuid1 == uuid2);
            }

            THEN("their string representations may not be equal") {
                REQUIRE(string(uuid1) != string(uuid2));
            }

            WHEN("one is converted into a flatbuffer") {
                scheme::cryptography::UUID buffer = uuid1.toScheme();

                AND_WHEN("it is converted back into a UUID") {
                    UUID reconstructed_uuid(&buffer);

                    THEN("they should be equal") {
                        REQUIRE(uuid1 == reconstructed_uuid);
                    }
                }
            }
        }
    }
#endif // UNIT_TESTING

}