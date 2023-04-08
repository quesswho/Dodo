#pragma once

#include <Dodo.h>

enum BlockType {
	AIR,
	GRASS,
	DIRT
};

class Block {
public:
	Block(BlockType type)
		: m_Type(type)
	{}
	BlockType m_Type;
};