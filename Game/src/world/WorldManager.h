#pragma once

#include <Dodo.h>

#include "World.h"
#include "WorldRenderer.h"

class WorldManager {
public:

	WorldManager(Ref<ResourceManager> resourceManager, Dodo::Math::FreeCamera* camera);

	Ref<ResourceManager> m_ResourceManager;
	Ref<World> m_World;
	Ref<WorldRenderer> m_WorldRenderer;

	void Draw();
};