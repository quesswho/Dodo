#include "WorldRenderer.h"



WorldRenderer::WorldRenderer(ResourceManager* resourceManager, Dodo::Math::FreeCamera* camera)
    : m_ResourceManager(resourceManager), m_Camera(camera)
{}

void WorldRenderer::Draw(World* world) {
    
    for (Ref<Chunk> chunk : world->m_Chunks) {
        RenderChunk(chunk);
    }
}

void WorldRenderer::RenderChunk(Ref<Chunk> chunk) {

    m_ResourceManager->m_GrassMaterial->SetUniform("u_Camera", m_Camera->GetCameraMatrix());
    m_ResourceManager->m_GrassMaterial->Bind();
    for(int i = 0; i < 4096; i++) {
        Ref<Block> block = chunk->m_Blocks[i];
        if (block->m_Type == BlockType::AIR) continue;
        byte face = chunk->m_VisibleFace[i];
        if (face == 0) continue;
        m_ResourceManager->m_GrassMaterial->SetUniform("u_Model", Dodo::Math::Mat4::Translate(Dodo::Math::Vec3(block->m_Pos.x + (chunk->m_ChunkPos.x << 4), block->m_Pos.y, block->m_Pos.z + (chunk->m_ChunkPos.y << 4))));
        m_ResourceManager->m_FaceIBuffer->Bind();
        if (face & 1) {
            m_ResourceManager->m_LeftVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
        if (face & 1 << 1) {
            m_ResourceManager->m_RightVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
        if (face & 1 
            << 2) {
            m_ResourceManager->m_BottomVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
        if (face & 1 << 3) {
            m_ResourceManager->m_TopVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
        if (face & 1 << 4) {
            m_ResourceManager->m_BackVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
        if (face & 1 << 5) {
            m_ResourceManager->m_FrontVBuffer->Bind();
            Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_ResourceManager->m_FaceIBuffer->GetCount());
        }
    }
}