#ifndef HoMesh_API_PROVIDER_HPP
#define HoMesh_API_PROVIDER_HPP

#include "keys.hpp"
#include "storage.hpp"
#include "network.hpp"
#include "time.hpp"

struct APIProvider {
    shared_ptr<KeyProvider> key;
    shared_ptr<StorageProvider> stor;
    shared_ptr<NetworkProvider> net;
    shared_ptr<RelativeTimeProvider> time;
};

#endif //HoMesh_API_PROVIDER_HPP
