#include "WorldGeneration.h"

Ref<Chunk> WorldGeneration::GenerateChunk(TVec2<int> chunkpos) {
	std::array<Ref<Block>, 4096> m_Blocks;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				m_Blocks[(x << 8) + (y << 4) + z] = std::make_shared<Block>(BlockType::GRASS);
			}
		}
	}
	return std::make_shared<Chunk>(chunkpos, m_Blocks);
}