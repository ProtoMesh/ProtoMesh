#ifndef HoMesh_STORAGE_HPP
#define HoMesh_STORAGE_HPP

#include <string>
#include <vector>

using namespace std;

class StorageProvider {
public:
    void set(string key, string value) {
        vector<uint8_t> vec(value.begin(), value.end());
        this->set(key, vec);
    }

    string get_str(string key) {
        vector<uint8_t> vec(this->get(key));
        string value(vec.begin(), vec.end());
        return value;
    }

    virtual void set(string key, vector<uint8_t> value)= 0;
    virtual vector<uint8_t> get(string key)= 0;
    virtual bool has(string key)= 0;
};

#ifdef UNIT_TESTING
    class DummyStorageHandler : public StorageProvider  {
        inline void set(string key, vector<uint8_t> value) {};
        inline vector<uint8_t> get(string key) { return {}; };
        inline bool has(string key) { return false; };
    };
#endif

#endif //HoMesh_STORAGE_HPP
