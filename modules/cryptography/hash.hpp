//
// Created by Til Blechschmidt on 06.11.17.
//

#ifndef PROTOMESH_HASH_HPP
#define PROTOMESH_HASH_HPP

#include <string>
#include <vector>

#include "sha512.hpp"

using namespace std;

#define HASH vector<uint8_t>

#define MAKE_HASHABLE(type, ...) \
    namespace std {\
        template<> struct hash<type> {\
            std::size_t operator()(const type &t) const {\
                std::size_t ret = 0;\
                ProtoMesh::cryptography::hash::hash_combine(ret, __VA_ARGS__);\
                return ret;\
            }\
        };\
    }

namespace ProtoMesh::cryptography::hash {
    string sha512(vector<uint8_t> message);
    HASH sha512Vec(vector<uint8_t> message);

    inline void hash_combine(std::size_t &seed) {}

    template<typename T, typename... Rest>
    inline void hash_combine(std::size_t &seed, const T &v, Rest... rest) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hash_combine(seed, rest...);
    }
}


#endif //PROTOMESH_HASH_HPP
