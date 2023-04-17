#include "Chunk.h"

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos) 
	: m_ChunkPos(chunkpos)
{
	for (int i = 0; i < 4096; i++) {
		m_Blocks[i] = std::make_shared<Block>(BlockType::AIR, BlockPos(i/256,i/16 % 16,i%16));
	}
}

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos, const std::array<Ref<Block>, 4096>& blocks)
	: m_ChunkPos(chunkpos), m_Blocks(blocks)
{}

BlockType Chunk::GetBlockType(int x, int y, int z) {
	if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16) {
		return m_Blocks[(x << 8) + (y << 4) + z]->m_Type;
	}
	else {
		return BlockType::AIR;
	}
}

void Chunk::SetBlockType(int x, int y, int z, BlockType type) {
	m_Blocks[(x << 8) + (y << 4) + z]->m_Type = type;
}
