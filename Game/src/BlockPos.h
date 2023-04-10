#pragma once

struct BlockPos {
	int x, y, z;
	BlockPos()
		: x(0),y(0),z(0)
	{}
	BlockPos(int x, int y, int z)
		: x(x), y(y), z(z)
	{}

};