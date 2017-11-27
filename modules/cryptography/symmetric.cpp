#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "symmetric.hpp"

#include <utility>

namespace ProtoMesh::cryptography::symmetric {

    Result<vector<uint8_t>, AESError> encrypt(vector<uint8_t> text, vector<uint8_t> key, vector<uint8_t> iv) {
        /// Prepare the IV
        if (iv.size() < IV_SIZE)
            return Err(AESError::IVTooSmall);
        iv.resize(IV_SIZE);

        /// Pad the text using zeros and write the padding size in the last byte
        auto paddingSize = static_cast<uint8_t>(text.size() % 16 == 0 ? 0 : 16 - (text.size() % 16));
        if (paddingSize) {
            for (int i = 0; i < (paddingSize - 1); ++i) text.push_back(0);
            text.push_back(paddingSize);
        }

        /// Prepare the output buffer
        vector<uint8_t> buffer = {0};
        buffer.resize(text.size(), 0);

        /// Encrypt the text
        AES_CBC_encrypt_buffer(buffer.data(), text.data(), static_cast<uint32_t>(text.size()), key.data(), iv.data());

        /// Append the IV to the buffer
        buffer.insert(buffer.end(),std::make_move_iterator(iv.begin()), std::make_move_iterator(iv.end()));

        return Ok(buffer);
    }

    Result<vector<uint8_t>, AESError> encrypt(vector<uint8_t> text, vector<uint8_t> key) {
        /// First create an instance of an engine.
        random_device rnd_device;
        /// Specify the engine and distribution.
        mt19937 mersenne_engine(rnd_device());
        uniform_int_distribution<unsigned int> dist(0, 255);
        auto gen = std::bind(dist, mersenne_engine);

        /// Generate a random IV
        vector<uint8_t> iv(IV_SIZE);
        generate(iv.begin(), iv.end(), gen);

        return encrypt(std::move(text), std::move(key), iv);
    }

    vector<uint8_t> decrypt(vector<uint8_t> ciphertext, vector<uint8_t> key) {
        /// Extract the IV from the end of the ciphertext and truncate the ciphertext
        vector<uint8_t> iv(ciphertext.end() - IV_SIZE, ciphertext.end());
        ciphertext.resize(max(ciphertext.size() - IV_SIZE, (size_t) 0));

        vector<uint8_t> buffer = {0};
        buffer.resize(ciphertext.size(), 0);

        AES_CBC_decrypt_buffer(buffer.data(), ciphertext.data(), static_cast<uint32_t>(ciphertext.size()), key.data(), iv.data());

        /// Remove any additional padding by looking at the last byte and checking if the last n bytes are equal to zero
        uint8_t paddingSize = buffer.back();
        size_t dataEnd = buffer.size() - paddingSize;
        /// [ buffer.size() - 2 ] because we want to ignore the padding size byte at the end
        for (size_t i = buffer.size() - 2; i > dataEnd; --i) {
            if (buffer[i] != 0) break;
            else if (i == dataEnd + 1) {
                buffer.resize(dataEnd);
                break;
            }
        }

        return buffer;
    }

#ifdef UNIT_TESTING

    SCENARIO("AES Cryptography", "[unit_test][module][cryptography][symmetric]") {
        GIVEN("An IV that is too short") {
            WHEN("it is attempted to encrypt a value") {
                THEN("it should fail") {
                    vector<uint8_t> input = {72, 195, 164, 115, 99, 104, 101, 110};
                    vector<uint8_t> iv; for (uint8_t i = 0; i < IV_SIZE/2; ++i) iv.push_back(i);
                    vector<uint8_t> key; for (uint8_t i = 0; i < IV_SIZE; ++i) key.push_back(i);

                    REQUIRE(encrypt(input, key, iv).isErr());
                }
            }
        }

        GIVEN("A sequence of bytes and an IV as well as a key") {
            vector<uint8_t> input = {72, 195, 164, 115, 99, 104, 101, 110};
            vector<uint8_t> iv; for (uint8_t i = 0; i < IV_SIZE; ++i) iv.push_back(i);
            vector<uint8_t> key; for (uint8_t i = 0; i < IV_SIZE; ++i) key.push_back(static_cast<uint8_t>(i * 2));

            WHEN("it is encrypted providing the IV") {
                vector<uint8_t> ciphertext = encrypt(input, key, iv).unwrap();

                AND_WHEN("it is decrypted using the correct key") {
                    vector<uint8_t> output = decrypt(ciphertext, key);
                    THEN("the output and the input should match") {
                        REQUIRE(output == input);
                    }
                }

                AND_WHEN("it is decrypted using the wrong key") {
                    vector<uint8_t> output = decrypt(ciphertext, iv);
                    THEN("it shouldn't match the input") {
                        REQUIRE_FALSE(output == input);
                    }
                }
            }

            WHEN("it is encrypted twice without providing the IV") {
                vector<uint8_t> ciphertext1 = encrypt(input, key).unwrap();
                vector<uint8_t> ciphertext2 = encrypt(input, key).unwrap();

                THEN("both ciphertexts may not be equal") {
                    REQUIRE(ciphertext1 != ciphertext2);
                }
            }
        }
    }
#endif //UNIT_TESTING
};