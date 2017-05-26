#ifndef UCL_STORAGE_HPP
#define UCL_STORAGE_HPP

#include <string>

using namespace std;

class StorageHandler {
public:
    virtual void set(string key, string value)= 0;
    virtual string get(string key)= 0;
    virtual bool has(string key)= 0;
};

#ifdef UNIT_TESTING
    class DummyStorageHandler : public StorageHandler  {
        inline void set(string key, string value) {};
        inline string get(string key) { return ""; };
        inline bool has(string key) { return false; };
    };
#endif

#endif //UCL_STORAGE_HPP
