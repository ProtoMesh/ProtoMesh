#ifndef OPEN_HOME_STORAGE_HPP
#define OPEN_HOME_STORAGE_HPP

#include <string>

using namespace std;

class StorageProvider {
public:
    virtual void set(string key, string value)= 0;
    virtual string get(string key)= 0;
    virtual bool has(string key)= 0;
};

#ifdef UNIT_TESTING
    class DummyStorageHandler : public StorageProvider  {
        inline void set(string key, string value) {};
        inline string get(string key) { return ""; };
        inline bool has(string key) { return false; };
    };
#endif

#endif //OPEN_HOME_STORAGE_HPP
