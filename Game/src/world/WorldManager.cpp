#include "WorldManager.h"

WorldManager::WorldManager(Ref<ResourceManager> resourceManager, Dodo::RenderAPI& renderAPI)
    : m_ResourceManager(resourceManager)
{
    m_WorldRenderer = std::make_shared<WorldRenderer>(m_ResourceManager);
    m_World = std::make_shared<World>(m_ResourceManager, m_WorldRenderer, renderAPI);
}

void WorldManager::Draw(const Dodo::Math::FreeCamera& camera, Dodo::RenderAPI& renderAPI)
{
    Dodo::FrameData data;
    data.camera = camera.GetCameraMatrix();
    renderAPI.SetFrameData(data);

    for (auto& chunk : m_World->m_Chunks) {
        m_WorldRenderer->RenderChunk(chunk.second, renderAPI);
    }
}