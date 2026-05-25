#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

Ref<Chunk> WorldGeneration::GenerateChunk(ChunkPos chunkpos)
{
    Ref<Chunk> chunk = std::make_shared<Chunk>(chunkpos);
    constexpr int worldHeight = 16 * VERTICAL_CHUNKS;
    constexpr float seaLevel  = worldHeight * 0.3f;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            const float scale      = 1.0f;
            const float biomescale = 0.5f;
            const float sandscale  = 3.0f;

            float noise = Dodo::Math::Noise::SumSimplex(
                ((chunkpos.x << 4) + x) * scale,
                ((chunkpos.y << 4) + z) * scale, 8, 0.35, 0.007) * 0.5f + 0.5f;

            float height_power = Dodo::Math::Noise::SumSimplex(
                ((chunkpos.x << 4) + x + 10000000) * biomescale,
                ((chunkpos.y << 4) + z + 10000000) * biomescale, 8, 0.35, 0.007) + 2.5f;

            const float sand_dunes = Dodo::Math::Noise::SumSimplex(
                ((chunkpos.x << 4) + x) * sandscale,
                ((chunkpos.y << 4) + z) * sandscale, 8, 0.35, 0.007) * 0.5f + 0.5f;

            float pow_noise = pow(noise, 2);
            float surface   = pow_noise * worldHeight;

            for (int y = 0; y < 16; y++) {
                int worldY = chunkpos.z * 16 + y;
                BlockType type;
                if ((float)worldY > surface) {
                    type = AIR;
                } else if (worldY < (int)surface - 4) {
                    type = STONE;
                } else if ((float)worldY + sand_dunes * 3.0f < seaLevel) {
                    type = SAND;
                } else if (worldY < (int)surface) {
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
