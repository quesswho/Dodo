#pragma once

#include "BlockPos.h"

#include <Dodo.h>

enum BlockType {
	AIR,
	GRASS,
	DIRT
};

struct Block {
	Block(BlockType type, BlockPos pos)
		: m_Type(type), m_Pos(pos)
	{}
	BlockType m_Type;
	BlockPos m_Pos;
};

struct RenderBlock {
	RenderBlock(BlockType type, BlockPos pos, byte face)
		: m_Type(type), m_Pos(pos), m_Face(face)
	{}

	BlockType m_Type;
	BlockPos m_Pos;
	byte m_Face;
};