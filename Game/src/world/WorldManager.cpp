#include "WorldManager.h"

WorldManager::WorldManager(Ref<ResourceManager> resourceManager, Dodo::Math::FreeCamera* camera)
	: m_ResourceManager(resourceManager)
{
	m_WorldRenderer = std::make_shared<WorldRenderer>(m_ResourceManager, camera);
	m_World = std::make_shared<World>(m_ResourceManager, m_WorldRenderer);
}

void WorldManager::Draw() {
	for (auto& chunk : m_World->m_Chunks) {
		m_WorldRenderer->RenderChunk(chunk.second);
	}
}