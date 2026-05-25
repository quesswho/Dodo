#pragma once
#include <Dodo.h>

#include "../ResourceManager.h"
#include "Chunk.h"

using namespace Dodo::Math;

enum class BiomeType {
    Desert, Savanna, Jungle, Plains, Forest, Swamp, Tundra, Taiga
};

struct BiomeParams {
    BlockType surfaceBlock;
    BlockType subsurfaceBlock;
    float     heightScale;
    float     heightBias;
    float     tempCenter;
    float     humidCenter;
};

class WorldGeneration {

  private:
    Ref<ResourceManager> m_ResourceManager;
    uint m_Seed;

    static BiomeParams GetBiomeParams(BiomeType biome);
    static BiomeParams BlendBiomes(float temp, float humid);

  public:
    WorldGeneration(uint seed, Ref<ResourceManager> resourceManager) : m_Seed(seed), m_ResourceManager(resourceManager)
    {}

    Ref<Chunk> GenerateChunk(ChunkPos chunkpos);
};