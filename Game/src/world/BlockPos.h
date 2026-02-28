#pragma once

#include <functional>

struct BlockPos {
	int x, y, z;
	BlockPos()
		: x(0),y(0),z(0)
	{}
	BlockPos(int x, int y, int z)
		: x(x), y(y), z(z)
	{}

};

struct ChunkPos {
	int x, y;
	ChunkPos()
		: x(0), y(0)
	{}
	ChunkPos(int x, int y)
		: x(x), y(y)
	{}


	// Allow hashing of ChunkPos
	bool operator==(const ChunkPos& otherPos) const
	{
		return (this->x == otherPos.x && this->y == otherPos.y);
	}

	struct HashFunction
	{
		size_t operator()(const ChunkPos& pos) const
		{
			size_t xHash = std::hash<int>()(pos.x);
			size_t yHash = std::hash<int>()(pos.y) << 1;
			return xHash ^ yHash;
		}
	};
};