#include "crypto.hpp"

namespace Crypto {
    string generateUUID() {
        random_device rd;
        default_random_engine generator(rd());
        uniform_int_distribution<uint32_t> distribution(0, numeric_limits<uint32_t>::max());

        stringstream ss;
        ss << hex << nouppercase << setfill('0');

        uint32_t a = distribution(generator);
        uint32_t b = distribution(generator);
        uint32_t c = distribution(generator);
        uint32_t d = distribution(generator);

        ss << setw(8) << (a) << '-';
        ss << setw(4) << (b >> 16) << '-';
        ss << setw(4) << (b & 0xFFFF) << '-';
        ss << setw(4) << (c >> 16) << '-';
        ss << setw(4) << (c & 0xFFFF);
        ss << setw(8) << d;

        return ss.str();
    }

    namespace hash {
        string sha512(string message) { return sw::sha512::calculate(message); }
        HASH sha512Vec(string message) {
            string sha512 = Crypto::hash::sha512(message);
            vector<uint8_t> sha512Vector(sha512.begin(), sha512.end());
            return sha512Vector;
        }
    }

    namespace serialize {
        string uint8ArrToString(uint8_t *arr, unsigned int len) {
            stringstream ss;
            ss << hex << nouppercase << setfill('0');
            for (int i = 0; i < len; ++i)
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
    }
}