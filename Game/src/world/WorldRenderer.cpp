#include "WorldRenderer.h"



WorldRenderer::WorldRenderer(ResourceManager* resourceManager, Dodo::Math::FreeCamera* camera)
    : m_ResourceManager(resourceManager), m_Camera(camera)
{}

void WorldRenderer::RenderChunk(Ref<Chunk> chunk) {
    m_ResourceManager->m_TextureAtlas->SetUniform("u_Camera", m_Camera->GetCameraMatrix());
    m_ResourceManager->m_TextureAtlas->SetUniform("u_Model", Dodo::Math::Mat4::Translate(Dodo::Math::Vec3((chunk->m_ChunkPos.x << 4), 0, (chunk->m_ChunkPos.y << 4))));
    m_ResourceManager->m_TextureAtlas->Bind();
    chunk->m_Vertbuffer->Bind();
    chunk->m_Indexbuffer->Bind();
    Dodo::Application::s_Application->m_RenderAPI->DrawIndices(chunk->m_Indexbuffer->GetCount());
}

void WorldRenderer::UpdateChunk(Ref<Chunk> chunk) {
    std::vector<FaceData> faces;
    std::vector<uint> indices;
    uint i = 0;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                BlockType type = chunk->GetBlockType(x, y, z);
                if (type != BlockType::AIR) {
                    if (chunk->GetBlockType(x, y, z + 1) == BlockType::AIR) { // front
                        faces.push_back(m_ResourceManager->GetFrontFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (chunk->GetBlockType(x, y, z - 1) == BlockType::AIR) { // back
                        faces.push_back(m_ResourceManager->GetBackFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }

                    if (chunk->GetBlockType(x - 1, y, z) == BlockType::AIR) { // right
                        faces.push_back(m_ResourceManager->GetLeftFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (chunk->GetBlockType(x + 1, y, z) == BlockType::AIR) { // left
                        faces.push_back(m_ResourceManager->GetRightFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }

                    if (chunk->GetBlockType(x, y - 1, z) == BlockType::AIR) { // bottom
                        faces.push_back(m_ResourceManager->GetBottomFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (chunk->GetBlockType(x, y + 1, z) == BlockType::AIR) { // top
                        faces.push_back(m_ResourceManager->GetTopFace(type, BlockPos(x, y, z)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
            }
        }
    }
    chunk->m_Vertbuffer = std::make_shared<Dodo::VertexBuffer>((float*)&faces[0], faces.size() * sizeof(FaceData), Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    chunk->m_Indexbuffer = std::make_shared<Dodo::IndexBuffer>(indices.data(), indices.size());
}