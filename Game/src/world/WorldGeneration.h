#pragma once
#include <Dodo.h>

#include "../ResourceManager.h"
#include "Chunk.h"

using namespace Dodo::Math;
class WorldGeneration {

  private:
    Ref<ResourceManager> m_ResourceManager;
    uint m_Seed;

  public:
    WorldGeneration(uint seed, Ref<ResourceManager> resourceManager) : m_Seed(seed), m_ResourceManager(resourceManager)
    {}

    Ref<Chunk> GenerateChunk(ChunkPos chunkpos);

    // void BuildBlock()
};