#pragma once

#include "Block.h"


class Chunk {
private:
	void SetBlockType(int x, int y, int z, BlockType type);
public:
	BlockType GetBlockType(int x, int y, int z);

	Chunk(Dodo::Math::TVec2<int> chunkpos);
	Chunk(Dodo::Math::TVec2<int> chunkpos, const std::array<Ref<Block>, 4096>& blocks);


	Dodo::Math::TVec2<int> m_ChunkPos;

	// 16x16x16 blocks
	std::array<Ref<Block>, 4096> m_Blocks;
	std::array<byte, 4096> m_VisibleFace;
	std::vector<RenderBlock> m_VisibleBlocks;
	Ref<Dodo::VertexBuffer> m_Vertbuffer;
	Ref<Dodo::IndexBuffer> m_Indexbuffer;

};