#ifndef PROTOMESH_SERIALIZATION_HPP
#define PROTOMESH_SERIALIZATION_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

namespace ProtoMesh::cryptography::serialization {
    string uint8ArrToString(uint8_t *arr, unsigned long len);
    vector<uint8_t> stringToUint8Array(string hex);
}


#endif //PROTOMESH_SERIALIZATION_HPP
