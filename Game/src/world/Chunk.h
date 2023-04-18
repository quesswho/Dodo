#pragma once

#include <Dodo.h>

#include "Block.h"

class Chunk {
private:
	void SetBlockType(int x, int y, int z, BlockType type);
public:
	BlockType GetBlockType(int x, int y, int z);

	Chunk(ChunkPos chunkpos);
	Chunk(ChunkPos chunkpos, const std::array<Ref<Block>, 4096>& blocks);


	ChunkPos m_ChunkPos;

	// 16x16x16 blocks
	std::array<Ref<Block>, 4096> m_Blocks;
	Ref<Dodo::VertexBuffer> m_Vertbuffer;
	Ref<Dodo::IndexBuffer> m_Indexbuffer;
};