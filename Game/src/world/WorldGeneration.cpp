#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

Ref<Chunk> WorldGeneration::GenerateChunk(ChunkPos chunkpos)
{
    Ref<Chunk> chunk = std::make_shared<Chunk>(chunkpos);
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            const float scale = 1.0f;
            const float biomescale = 0.5f;
            const float sandscale = 3.0f;

            float noise = Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x) * scale,
                                                        ((chunkpos.y << 4) + z) * scale, 8, 0.35, 0.007) *
                              0.5 +
                          0.5;
            float height_power =
                Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x + 10000000) * biomescale,
                                              ((chunkpos.y << 4) + z + 10000000) * biomescale, 8, 0.35, 0.007) +
                2.5;
            const float sand_dunes =
                Dodo::Math::Noise::SumSimplex(((chunkpos.x << 4) + x) * sandscale,
                                              ((chunkpos.y << 4) + z) * sandscale, 8, 0.35, 0.007) *
                    0.5 +
                0.5;
            float pow_noise = pow(noise, height_power);
            for (int y = 0; y < 16; y++) {
                BlockType type;
                if (pow_noise < y / 16.0) {
                    type = AIR;
                } else if (y - sand_dunes * 3 < 2) {
                    type = SAND;
                } else if (pow_noise >= (y + 1) / 16.0) {
                    type = DIRT;
                } else {
                    type = GRASS;
                }
                chunk->SetBlockType(x, y, z, type);
            }
        }
    }
    return chunk;
}
