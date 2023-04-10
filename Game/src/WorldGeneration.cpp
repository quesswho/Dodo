#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

Ref<Chunk> WorldGeneration::GenerateChunk(TVec2<int> chunkpos) {
	std::array<Ref<Block>, 4096> m_Blocks;
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			const float scale = 10;
			float noise = Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x) * scale, ((chunkpos.y << 4) + z) * scale, 25,0.5,0.007)*0.5+0.5;
			for (int y = 0; y < 16; y++) {
				if (noise < y / 16.0) {
					m_Blocks[(x << 8) + (y << 4) + z] = std::make_shared<Block>(BlockType::AIR, BlockPos(x,y,z));
				} else {
					m_Blocks[(x << 8) + (y << 4) + z] = std::make_shared<Block>(BlockType::GRASS, BlockPos(x, y, z));
				}
			}
		}
	}
	return std::make_shared<Chunk>(chunkpos, m_Blocks);
}