#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

Ref<Chunk> WorldGeneration::GenerateChunk(TVec2<int> chunkpos) {
	std::array<Ref<Block>, 4096> m_Blocks;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				float test = Dodo::Math::Noise::Simplex(((chunkpos.x << 4) + x + z*x * y*76492.12+10.123) * 0.321, ((chunkpos.y << 4) + z * y + 0.953) * 0.321);
				//float test = Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x + y + 0.123) * 0.5, ((chunkpos.y << 4) + z * y + 0.953) * 0.5, 16, 0.25, 0.007);
				if (test > 0.8) {
					m_Blocks[(x << 8) + (y << 4) + z] = std::make_shared<Block>(BlockType::GRASS);
				} else {
					m_Blocks[(x << 8) + (y << 4) + z] = std::make_shared<Block>(BlockType::AIR);
				}
			}
		}
	}
	return std::make_shared<Chunk>(chunkpos, m_Blocks);
}