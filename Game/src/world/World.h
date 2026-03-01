#pragma once

#include "Chunk.h"
#include "WorldGeneration.h"
#include "WorldRenderer.h"

#include <array>
#include <memory>

class World {
  private:
    Ref<WorldGeneration> m_WorldGen;
    Ref<WorldRenderer> m_WorldRenderer;
    Ref<ResourceManager> m_ResourceManager;

  public:
    World(Ref<ResourceManager> resourceManager, Ref<WorldRenderer> worldRenderer);

    std::unordered_map<ChunkPos, Ref<Chunk>, ChunkPos::HashFunction> m_Chunks;

    void UpdateChunk(ChunkPos chunkpos);
    void Draw();

    BlockType GetBlockType(int x, int y, int z);
};