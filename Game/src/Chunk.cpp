#include "Chunk.h"

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos) 
	: m_ChunkPos(chunkpos)
{
	for (int i = 0; i < 4096; i++) {
		m_Blocks[i] = std::make_shared<Block>(BlockType::AIR);
	}
	UpdateVisibleFaces();
}

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos, std::array<Ref<Block>, 4096> blocks)
	: m_ChunkPos(chunkpos), m_Blocks(blocks)
{
	UpdateVisibleFaces();
}


void Chunk::UpdateVisibleFaces() {
	memset(&m_VisibleFace, -1, 4096);
	for (int x = 1; x < 15; x++) {
		for (int y = 1; y < 15; y++) {
			for (int z = 1; z < 15; z++) {
				if (GetBlockType(x, y, z) != BlockType::AIR) {
					int temp = (GetBlockType(x - 1, y, z) == BlockType::AIR);
					int index = (x << 8) + (y << 4) + z;
					m_VisibleFace[(x << 8) + (y << 4) + z] =
						(GetBlockType(x - 1, y, z) == BlockType::AIR) |
						(GetBlockType(x + 1, y, z) == BlockType::AIR << 1) |
						(GetBlockType(x, y - 1, z) == BlockType::AIR << 2) |
						(GetBlockType(x, y + 1, z) == BlockType::AIR << 3) |
						(GetBlockType(x, y, z - 1) == BlockType::AIR << 4) |
						(GetBlockType(x, y, z + 1) == BlockType::AIR << 5);
				}
			}
		}
	}
}

BlockType Chunk::GetBlockType(int x, int y, int z) {
	return m_Blocks[(x << 8) + (y << 4) + z]->m_Type;
}

void Chunk::SetBlockType(int x, int y, int z, BlockType type) {
	m_Blocks[(x << 8) + (y << 4) + z]->m_Type = type;
}
