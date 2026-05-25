#include "WorldRenderer.h"

WorldRenderer::WorldRenderer(Ref<ResourceManager> resourceManager) : m_ResourceManager(resourceManager) {}

void WorldRenderer::RenderChunk(Ref<Chunk> chunk, Dodo::RenderAPI& renderAPI)
{
    Dodo::Math::Mat4 model = Dodo::Math::Mat4::Translate(Dodo::Math::Vec3((chunk->m_ChunkPos.x << 4), 0, (chunk->m_ChunkPos.y << 4)));
    
    Dodo::Math::Mat3 model3x3(Dodo::Math::Vec3(model.m_Columns[0].x, model.m_Columns[0].y, model.m_Columns[0].z),
    Dodo::Math::Vec3(model.m_Columns[1].x, model.m_Columns[1].y, model.m_Columns[1].z),
    Dodo::Math::Vec3(model.m_Columns[2].x, model.m_Columns[2].y, model.m_Columns[2].z));
    
    Dodo::DrawData drawData;
    drawData.model = model;
    drawData.normalMatrix = Dodo::Math::Mat3::Transpose(Dodo::Math::Mat3::Inverse(model3x3));

    renderAPI.SetDrawData(drawData);
    renderAPI.BindVertexBuffer(chunk->m_Vertbuffer);
    renderAPI.BindIndexBuffer(chunk->m_Indexbuffer);
    renderAPI.DrawIndices(chunk->m_Indexbuffer->GetCount());
}