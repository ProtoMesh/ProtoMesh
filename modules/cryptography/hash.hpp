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


namespace ProtoMesh::cryptography::hash {
    string sha512(vector<uint8_t> message);
    HASH sha512Vec(vector<uint8_t> message);
}


#endif //PROTOMESH_HASH_HPP
