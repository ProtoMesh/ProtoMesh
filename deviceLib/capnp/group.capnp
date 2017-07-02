@0xf5660fc439f3dbcc;

using import "generic.capnp".UUID;
using import "node.capnp".Node;

struct GroupNode {
    isAdmin @0 :Bool;

    node @1 :Node;
}

struct Group {
    id @0 :UUID;
    name @1 :Text;

    children @2 :List(GroupNode);
}