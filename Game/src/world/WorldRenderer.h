#pragma once

#include <Dodo.h>

#include "Chunk.h"
#include "../ResourceManager.h"

class WorldRenderer {
private:
	Dodo::Math::FreeCamera* m_Camera;

public:
	WorldRenderer(ResourceManager* resourceManager, Dodo::Math::FreeCamera* camera);

	void RenderChunk(Ref<Chunk> chunk);
	void UpdateChunk(Ref<Chunk> chunk);

	ResourceManager* m_ResourceManager;
};