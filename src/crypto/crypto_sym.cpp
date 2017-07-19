#include "crypto.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

namespace Crypto {
    namespace sym {
        Result<vector<uint8_t>, AESError> encrypt(vector<uint8_t> text, vector<uint8_t> key, vector<uint8_t> iv) {
            /// Prepare the IV
            if (iv.size() < IV_SIZE)
                return Err(AESError(AESError::Kind::IVTooSmall, "The provided IV didn't have the required minimum size of " + IV_SIZE));
            iv.resize(IV_SIZE);

            /// Pad the text using zeros and write the padding size in the last byte
            uint8_t paddingSize = static_cast<uint8_t>(text.size() % 16 == 0 ? 0 : 16 - (text.size() % 16));
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
    }
}

#ifdef UNIT_TESTING
    SCENARIO("AES Cryptography", "[crypto][sym][aes]") {

    }
#endif //UNIT_TESTING