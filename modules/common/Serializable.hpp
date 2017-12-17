#ifndef PROTOMESH_SERIALIZABLE_HPP
#define PROTOMESH_SERIALIZABLE_HPP

#include <vector>

using namespace std;

#include "result.h"

enum class DeserializationError {
    INVALID_IDENTIFIER,
    INVALID_BUFFER,
    INVALID_PUB_KEY,
    INVALID_ORIGIN_KEY,
    SIGNATURE_SIZE_MISMATCH,
    UNIMPLEMENTED
};

template <class T>
class Serializable {
public:
    virtual ~Serializable()= default;

    virtual vector<uint8_t> serialize() const = 0;
    static Result<T, DeserializationError> fromBuffer(vector<uint8_t> buffer) {
        return Err(DeserializationError::UNIMPLEMENTED);
    };
};


#endif //PROTOMESH_SERIALIZABLE_HPP
