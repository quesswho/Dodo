#include "WorldRenderer.h"

WorldRenderer::WorldRenderer(Ref<ResourceManager> resourceManager) : m_ResourceManager(resourceManager) {}

void WorldRenderer::RenderChunk(Ref<Chunk> chunk, Dodo::RenderAPI& renderAPI)
{
    m_ResourceManager->m_TextureAtlas->Bind(renderAPI);
    renderAPI.SetDrawData(
        {Dodo::Math::Mat4::Translate(Dodo::Math::Vec3((chunk->m_ChunkPos.x << 4), 0, (chunk->m_ChunkPos.y << 4)))});
    renderAPI.BindVertexBuffer(chunk->m_Vertbuffer);
    renderAPI.BindIndexBuffer(chunk->m_Indexbuffer);
    renderAPI.DrawIndices(chunk->m_Indexbuffer->GetCount());
}