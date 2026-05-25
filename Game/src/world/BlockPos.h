#pragma once

#include <functional>

struct BlockPos {
    int x, y, z;
    BlockPos() : x(0), y(0), z(0) {}
    BlockPos(int x, int y, int z) : x(x), y(y), z(z) {}
};

struct ChunkPos {
    int x, y, z;  // x=grid X (horizontal), y=grid Y (depth), z=grid Z (vertical, up)
    ChunkPos() : x(0), y(0), z(0) {}
    ChunkPos(int x, int y, int z = 0) : x(x), y(y), z(z) {}

    bool operator==(const ChunkPos& o) const { return x == o.x && y == o.y && z == o.z; }

    struct HashFunction {
        size_t operator()(const ChunkPos& pos) const
        {
            size_t seed = 0x75e2e1e735d9a5c7ULL;
            seed ^= std::hash<int>()(pos.x) + 0x9e3779b97f4a7c15ULL + (seed << 12) + (seed >> 4);
            seed ^= std::hash<int>()(pos.y) + 0x9e3779b97f4a7c15ULL + (seed << 12) + (seed >> 4);
            seed ^= std::hash<int>()(pos.z) + 0x9e3779b97f4a7c15ULL + (seed << 12) + (seed >> 4);
            return seed;
        }
    };
};