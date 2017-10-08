#ifndef HoMesh_CONST_HPP
#define HoMesh_CONST_HPP

/// Networking
#define MULTICAST_NETWORK "235.17.10.20"

#define REGISTRY_PORT   1337
#define DEVICE_PORT     1338


/// Internal constants
#define RECV_OK     0
#define RECV_ERR    1


/// Registry synchronization
#define REGISTRY_BROADCAST_INTERVAL_MIN 1000
#define REGISTRY_BROADCAST_INTERVAL_MAX 30000

#define REGISTRY_SYNC_TIMEOUT 20000


/// Registry names
#define NODES_REGISTRY  "network::nodes"
#define GROUPS_REGISTRY "network::groups"

#endif //HoMesh_CONST_HPP
