#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

Ref<Chunk> WorldGeneration::GenerateChunk(ChunkPos chunkpos) {
	std::array<Ref<Block>, 4096> m_Blocks;
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			const float scale = 1.0f;
			float noise = Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x) * scale, ((chunkpos.y << 4) + z) * scale, 25,0.35,0.007)*0.5+0.5;
			for (int y = 0; y < 16; y++) {
				if (noise < y / 16.0) {
					m_Blocks[(x << 8) + (y << 4) + z] = m_ResourceManager->GetBlock(AIR);
				}
				else {
					if (noise >= (y+1) / 16.0) {
						m_Blocks[(x << 8) + (y << 4) + z] = m_ResourceManager->GetBlock(DIRT);
					} else {
						m_Blocks[(x << 8) + (y << 4) + z] = m_ResourceManager->GetBlock(GRASS);
					}
				}
			}
		}
	}
	return std::make_shared<Chunk>(chunkpos, std::move(m_Blocks));
}