#ifndef UCL_STORAGE_HPP
#define UCL_STORAGE_HPP

#include <string>

using namespace std;

class StorageHandler {
public:
    virtual void save(string key, string value)= 0;
    virtual string read(string key)= 0;
};

#endif //UCL_STORAGE_HPP
