#pragma once

#include <Dodo.h>

#include "../ResourceManager.h"
#include "Chunk.h"

class WorldRenderer {
  private:
    Ref<ResourceManager> m_ResourceManager;
  public:
    WorldRenderer(Ref<ResourceManager> resourceManager);

    void RenderChunk(Ref<Chunk> chunk, Dodo::RenderAPI& renderAPI);
};