#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"

using namespace std;

class Registry {
    vector<RegistryEntry> entries;
    map<string, string> head;
public:
    Registry() {

    }

    void add(string key, string value) {

    }

    void updateHead() {
        head.clear();
//        for (const auto &entry : entries) {
//
//        }
    }
};


#endif //UCL_REGISTRY_HPP
