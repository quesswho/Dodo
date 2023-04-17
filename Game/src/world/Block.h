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

struct VertexData {
	VertexData(float x, float y, float z, float u, float v, float nx, float ny, float nz)
		: x(x), y(y), z(z), u(u), v(v), nx(nx), ny(ny), nz(nz)
	{}
	float x, y, z, u, v, nx, ny, nz;
};

struct FaceData {
	float verts[32];
};

struct RenderBlock {
	RenderBlock(BlockType type, BlockPos pos, byte face)
		: m_Type(type), m_Pos(pos), m_Face(face)
	{}

	BlockType m_Type;
	BlockPos m_Pos;
	byte m_Face;
};