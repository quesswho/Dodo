#pragma once

#include "BlockPos.h"

#include <Dodo.h>

enum BlockType {
	AIR,
	GRASS,
	DIRT
};

class Block {
public:
	Block(BlockType type, BlockPos pos)
		: m_Type(type), m_Pos(pos)
	{}
	BlockType m_Type;
	BlockPos m_Pos;
};