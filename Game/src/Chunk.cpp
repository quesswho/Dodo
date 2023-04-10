#include "Chunk.h"

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos) 
	: m_ChunkPos(chunkpos)
{
	for (int i = 0; i < 4096; i++) {
		m_Blocks[i] = std::make_shared<Block>(BlockType::AIR, BlockPos(i/256,i/16 % 16,i%16));
	}
	UpdateVisibleFaces();
}

Chunk::Chunk(Dodo::Math::TVec2<int> chunkpos, const std::array<Ref<Block>, 4096>& blocks)
	: m_ChunkPos(chunkpos), m_Blocks(blocks)
{
	UpdateVisibleFaces();
}


void Chunk::UpdateVisibleFaces() {
	memset(&m_VisibleFace, -1, 4096);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				if (GetBlockType(x, y, z) != BlockType::AIR) {
					int temp = (GetBlockType(x - 1, y, z) == BlockType::AIR);
					int index = (x << 8) + (y << 4) + z;
					m_VisibleFace[(x << 8) + (y << 4) + z] =
						(GetBlockType(x - 1, y, z) == BlockType::AIR) |      // back
						((GetBlockType(x + 1, y, z) == BlockType::AIR) << 1) | // front
						((GetBlockType(x, y - 1, z) == BlockType::AIR) << 2) | // top
						((GetBlockType(x, y + 1, z) == BlockType::AIR) << 3) | // bottom
						((GetBlockType(x, y, z - 1) == BlockType::AIR) << 4) | // left
						((GetBlockType(x, y, z + 1) == BlockType::AIR) << 5);	 // right
						
					//m_VisibleFace[(x << 8) + (y << 4) + z] = 255;
				}
			}
		}
	}
}

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
