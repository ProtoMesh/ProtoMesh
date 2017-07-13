#ifndef LUMOS_API_PROVIDER_HPP
#define LUMOS_API_PROVIDER_HPP

#include "keys.hpp"
#include "storage.hpp"
#include "network.hpp"
#include "time.hpp"

struct APIProvider {
    KeyProvider *key;
    StorageProvider *stor;
    NetworkProvider *net;
    REL_TIME_PROV_T time;
};

#endif //LUMOS_API_PROVIDER_HPP
