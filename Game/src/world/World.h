#pragma once

#include "Chunk.h"
#include "WorldGeneration.h"
#include "WorldRenderer.h"

#include "Core/Graphics/RenderAPI.h"

#include <array>
#include <memory>

class World {
  private:
    Ref<WorldGeneration> m_WorldGen;
    Ref<WorldRenderer> m_WorldRenderer;
    Ref<ResourceManager> m_ResourceManager;

  public:
    World(Ref<ResourceManager> resourceManager, Ref<WorldRenderer> worldRenderer, Dodo::RenderAPI& renderAPI);

    std::unordered_map<ChunkPos, Ref<Chunk>, ChunkPos::HashFunction> m_Chunks;
    bool m_MeshBuilt = false;

    void UpdateChunk(ChunkPos chunkpos, Dodo::RenderAPI& renderAPI);
    void BuildMeshes(Dodo::RenderAPI& renderAPI);
    void Draw();

    BlockType GetBlockType(int x, int y, int z);
};