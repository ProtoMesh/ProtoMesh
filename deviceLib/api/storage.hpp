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

#endif //UCL_STORAGE_HPP
