@0xd387d66dcfe020b3;

struct UUID {
    # Since a UUID consists of 128-bits we just represent it with two 64-bit integers
    p1 @0 :UInt64;
    p2 @1 :UInt64;
}