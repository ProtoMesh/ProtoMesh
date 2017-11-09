//
// Created by Til Blechschmidt on 06.11.17.
//

#ifndef PROTOMESH_UUID_HPP
#define PROTOMESH_UUID_HPP

#include <random>
#include <array>
#include <utility>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "cryptography/uuid_generated.h"

using namespace std;

namespace ProtoMesh::cryptography {

    class UUID {
    public:
        uint32_t a = 0, b = 0, c = 0, d = 0;

        void generateRandom();

        static UUID Empty() { return {0, 0, 0, 0}; }
        UUID(uint32_t a, uint32_t b, uint32_t c, uint32_t d) : a(a), b(b), c(c), d(d) {};
        UUID();
        explicit UUID(const scheme::cryptography::UUID *id);

        scheme::cryptography::UUID toScheme() const;
        explicit operator string() const;

        inline tuple <uint32_t, uint32_t, uint32_t, uint32_t> tie() const { return std::tie(a, b, c, d); }
        inline bool operator==(const UUID &other) const { return this->tie() == other.tie(); }
        inline bool operator!=(const UUID &other) const { return this->tie() != other.tie(); }
        inline bool operator>(const UUID &other) const { return this->tie() > other.tie(); }
        inline bool operator<(const UUID &other) const { return this->tie() < other.tie(); }
    };

    inline std::ostream &operator<<(std::ostream &out, const UUID &uid) {
        out << string(uid);
        return out;
    }

}


#endif //PROTOMESH_UUID_HPP
