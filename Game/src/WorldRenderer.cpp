#include "WorldRenderer.h"

static uint indices[]{
    // front
    0,  1,  2,
    2,  3,  0,
    // top
    4,  5,  6,
    6,  7,  4,
    // back
    8,  9, 10,
    10, 11,  8,
    // bottom
    12, 13, 14,
    14, 15, 12,
    // left
    16, 17, 18,
    18, 19, 16,
    // right
    20, 21, 22,
    22, 23, 20,
};

float cube_vertices[] = {
    // front
     -0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0,
     0.5, -0.5,  0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0, 0.0, 1.0,
    -0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
    // top
    -0.5,  0.5,  0.5, 0.0, 0.0, 0.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0,
    // back
     0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0,
    -0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0,
    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0,
     0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 0.0, -1.0,
     // bottom
     -0.5, -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0,
      0.5, -0.5, -0.5, 1.0, 0.0, 0.0, -1.0, 0.0,
      0.5, -0.5,  0.5, 1.0, 1.0, 0.0, -1.0, 0.0,
     -0.5, -0.5,  0.5, 0.0, 1.0, 0.0, -1.0, 0.0,
     // left
     -0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0,
     -0.5, -0.5,  0.5, 1.0, 0.0, -1.0, 0.0, 0.0,
     -0.5,  0.5,  0.5, 1.0, 1.0, -1.0, 0.0, 0.0,
     -0.5,  0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0,
     // right
      0.5, -0.5,  0.5, 0.0, 0.0, 1.0, 0.0, 0.0,
      0.5, -0.5, -0.5, 1.0, 0.0, 1.0, 0.0, 0.0,
      0.5,  0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0,
      0.5,  0.5,  0.5, 0.0, 1.0, 1.0, 0.0, 0.0,
};

WorldRenderer::WorldRenderer(Dodo::Math::FreeCamera* camera)
    : m_Camera(camera)
{
    Ref<Dodo::Texture> texture = std::make_shared<Dodo::Texture>("res/texture/grass.png", 0, Dodo::TextureSettings(Dodo::TextureFilter::FILTER_MIN_MAG_MIP_NEAREST));
    m_GrassMaterial = std::make_shared<Dodo::Material>(Dodo::Shader::CreateFromPath("block", "res/shader/block.glsl"), texture);
    m_CubeVBuffer = std::make_shared<Dodo::VertexBuffer>(cube_vertices, 24 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_CubeIBuffer = std::make_shared<Dodo::IndexBuffer>(indices, 36);
}

void WorldRenderer::Draw(World* world) {
    
    for (Ref<Chunk> chunk : world->m_Chunks) {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    if (chunk->m_VisibleFace[(x << 8) + (y << 4) + z] > 0) {
                        m_GrassMaterial->SetUniform("u_Camera", m_Camera->GetCameraMatrix());
                        m_GrassMaterial->SetUniform("u_Model", Dodo::Math::Mat4::Translate(Dodo::Math::Vec3(x+(chunk->m_ChunkPos.x << 4), y, z + (chunk->m_ChunkPos.y << 4))));
                        m_GrassMaterial->Bind();
                        m_CubeVBuffer->Bind();
                        m_CubeIBuffer->Bind();
                        Dodo::Application::s_Application->m_RenderAPI->DrawIndices(m_CubeIBuffer->GetCount());
                    }
                }
            }
        }
    }
}