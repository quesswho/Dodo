#pragma once

#include <Dodo.h>

#include "World.h"
#include "../ResourceManager.h"

class WorldRenderer {
private:
	Dodo::Math::FreeCamera* m_Camera;
	ResourceManager* m_ResourceManager;

public:
	WorldRenderer(ResourceManager* m_ResourceManager, Dodo::Math::FreeCamera* camera);

	void Draw(World* world);
	void RenderChunk(Ref<Chunk> chunk);
};