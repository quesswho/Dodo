#include "WorldGeneration.h"
#include "Core/Math/Random/Noise.h"

BiomeParams WorldGeneration::GetBiomeParams(BiomeType biome)
{
    switch (biome) {
        case BiomeType::Desert:  return { SAND,  SAND,  0.50f,  0.0f, 0.835f, 0.165f };
        case BiomeType::Savanna: return { GRASS, DIRT,  0.75f,  0.0f, 0.835f, 0.500f };
        case BiomeType::Jungle:  return { GRASS, DIRT,  1.40f,  0.0f, 0.835f, 0.835f };
        case BiomeType::Plains:  return { GRASS, DIRT,  0.70f,  0.0f, 0.500f, 0.165f };
        case BiomeType::Forest:  return { GRASS, DIRT,  0.90f,  0.0f, 0.500f, 0.500f };
        case BiomeType::Swamp:   return { GRASS, DIRT,  0.55f, -1.5f, 0.500f, 0.835f };
        case BiomeType::Tundra:  return { STONE, STONE, 0.60f,  0.0f, 0.165f, 0.165f };
        default:                 return { GRASS, DIRT,  0.80f,  0.0f, 0.165f, 0.500f }; // Taiga
    }
}

BiomeParams WorldGeneration::BlendBiomes(float temp, float humid)
{
    static const BiomeType allBiomes[] = {
        BiomeType::Desert, BiomeType::Savanna, BiomeType::Jungle,
        BiomeType::Plains, BiomeType::Forest,  BiomeType::Swamp,
        BiomeType::Tundra, BiomeType::Taiga
    };
    constexpr int   N       = 8;
    constexpr float EPSILON = 1e-6f;

    BiomeParams params[N];
    float weights[N];
    float weightSum   = 0.0f;
    int   dominantIdx = 0;

    for (int i = 0; i < N; i++) {
        params[i] = GetBiomeParams(allBiomes[i]);
        float dt    = temp  - params[i].tempCenter;
        float dh    = humid - params[i].humidCenter;
        float dist2 = dt * dt + dh * dh;
        if (dist2 < EPSILON) return params[i];
        float dist  = sqrtf(dist2);
        weights[i]  = 1.0f / (dist * dist * dist);
        weightSum  += weights[i];
    }

    float heightScale = 0.0f, heightBias = 0.0f;
    for (int i = 0; i < N; i++) {
        float w     = weights[i] / weightSum;
        heightScale += params[i].heightScale * w;
        heightBias  += params[i].heightBias  * w;
        if (weights[i] > weights[dominantIdx]) dominantIdx = i;
    }

    BiomeParams result = params[dominantIdx];
    result.heightScale = heightScale;
    result.heightBias  = heightBias;
    return result;
}

Ref<Chunk> WorldGeneration::GenerateChunk(ChunkPos chunkpos)
{
    Ref<Chunk> chunk = std::make_shared<Chunk>(chunkpos);
    constexpr int   worldHeight = 16 * VERTICAL_CHUNKS;
    constexpr float seaLevel    = worldHeight * 0.3f;

    float seedT = ((float)(uint32_t)(m_Seed * 2654435761u) / 4294967295.0f) * 100000.0f - 50000.0f;
    float seedH = ((float)(uint32_t)(m_Seed * 2654435761u ^ 0xDEADBEEFu) / 4294967295.0f) * 100000.0f - 50000.0f;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            const float wx = (float)((chunkpos.x << 4) + x);
            const float wz = (float)((chunkpos.y << 4) + z);

            float noise = Dodo::Math::Noise::SumSimplex(wx, wz, 8, 0.35f, 0.007f) * 0.5f + 0.5f;

            const float sand_dunes = Dodo::Math::Noise::SumSimplex(
                wx * 3.0f, wz * 3.0f, 8, 0.35f, 0.007f) * 0.5f + 0.5f;

            float temp  = Dodo::Math::Noise::SumSimplex(
                wx + seedT, wz + seedT, 4, 0.5f, 0.002f) * 0.5f + 0.5f;
            float humid = Dodo::Math::Noise::SumSimplex(
                wx + seedH, wz + seedH, 4, 0.5f, 0.003f) * 0.5f + 0.5f;

            BiomeParams biome = BlendBiomes(temp, humid);
            float surface = powf(noise, 2.0f) * (float)worldHeight * biome.heightScale + biome.heightBias;

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
                    type = biome.subsurfaceBlock;
                } else {
                    type = biome.surfaceBlock;
                }
                chunk->SetBlockType(x, y, z, type);
            }
        }
    }
    return chunk;
}
