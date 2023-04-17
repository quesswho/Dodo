#include "World.h"

World::World(ResourceManager* resourceManager, Dodo::Math::FreeCamera* camera) {
	m_WorldRenderer = std::make_shared<WorldRenderer>(resourceManager, camera);
	m_WorldGen = std::make_shared<WorldGeneration>(0);
	for (int x = -10; x <= 40; x++) {
		for (int y = -10; y <= 40; y++) {
			Ref<Chunk> chunk = m_WorldGen->GenerateChunk(TVec2<int>(x, y));
			m_WorldRenderer->UpdateChunk(chunk);
			m_Chunks.push_back(chunk);
		}
	}
}

void World::Draw() {
	for (Ref<Chunk> chunk : m_Chunks) {
		m_WorldRenderer->RenderChunk(chunk);
	}
}