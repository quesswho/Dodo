#pragma once

#include "Chunk.h"
#include "WorldGeneration.h"
#include "WorldRenderer.h"

#include <memory>
#include <array>

class World {
private:
	Ref<WorldGeneration> m_WorldGen;
	Ref<WorldRenderer> m_WorldRenderer;
public:
	World(ResourceManager* resourceManager, Dodo::Math::FreeCamera* camera);
	std::vector<Ref<Chunk>> m_Chunks;

	void Draw();
};