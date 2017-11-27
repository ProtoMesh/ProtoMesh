#ifndef PROTOMESH_SYMMETRIC_HPP
#define PROTOMESH_SYMMETRIC_HPP

#include <vector>

using namespace std;

#include "result.h"

#define AES256 1
extern "C" {
    /// Implemented @ lib/AES/aes.c
    void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
    void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
}


/// Since we use AES256 the IV may have a size of 32 * sizeof(uint8_t) = 256.
#define IV_SIZE 32

namespace ProtoMesh::cryptography::symmetric {
    enum class AESError {
        IVTooSmall
    };

    Result<vector<uint8_t>, AESError> encrypt(vector<uint8_t> text, vector<uint8_t> key, vector<uint8_t> iv);
    vector<uint8_t> decrypt(vector<uint8_t> ciphertext, vector<uint8_t> key);
};


#endif //PROTOMESH_SYMMETRIC_HPP
