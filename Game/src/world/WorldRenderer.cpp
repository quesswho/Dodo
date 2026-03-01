#include "WorldRenderer.h"

WorldRenderer::WorldRenderer(Ref<ResourceManager> resourceManager, Dodo::Math::FreeCamera *camera)
    : m_Camera(camera), m_ResourceManager(resourceManager)
{}

void WorldRenderer::RenderChunk(Ref<Chunk> chunk)
{
    m_ResourceManager->m_TextureAtlas->SetUniform("u_Camera", m_Camera->GetCameraMatrix());
    m_ResourceManager->m_TextureAtlas->SetUniform(
        "u_Model",
        Dodo::Math::Mat4::Translate(Dodo::Math::Vec3((chunk->m_ChunkPos.x << 4), 0, (chunk->m_ChunkPos.y << 4))));
    m_ResourceManager->m_TextureAtlas->Bind();
    chunk->m_Vertbuffer->Bind();
    chunk->m_Indexbuffer->Bind();
    Dodo::Application::s_Application->m_RenderAPI->DrawIndices(chunk->m_Indexbuffer->GetCount());
}