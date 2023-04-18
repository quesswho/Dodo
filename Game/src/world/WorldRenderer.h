#pragma once

#include <Dodo.h>

#include "Chunk.h"
#include "../ResourceManager.h"

class WorldRenderer {
private:
	Ref<ResourceManager> m_ResourceManager;
	Dodo::Math::FreeCamera* m_Camera;
public:
	WorldRenderer(Ref<ResourceManager> resourceManager, Dodo::Math::FreeCamera* camera);

	void RenderChunk(Ref<Chunk> chunk);
};