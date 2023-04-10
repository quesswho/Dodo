#include "ResourceManager.h"


float front_verts[] = {
    -0.5, -0.5,  0.5, 0.0, 0.0, 0.0, 0.0, 1.0,
     0.5, -0.5,  0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0, 0.0, 1.0,
    -0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1.0
};

float top_verts[] = {
    -0.5,  0.5,  0.5, 0.0, 0.0, 0.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0
};


float back_verts[] = {
     0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0,
    -0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0,
    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0,
     0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 0.0, -1.0
};


float bottom_verts[] = {
    -0.5, -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 0.0, 0.0, -1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0, -1.0, 0.0,
    -0.5, -0.5,  0.5, 0.0, 1.0, 0.0, -1.0, 0.0
};

float left_verts[] = {
    -0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0,
    -0.5, -0.5,  0.5, 1.0, 0.0, -1.0, 0.0, 0.0,
    -0.5,  0.5,  0.5, 1.0, 1.0, -1.0, 0.0, 0.0,
    -0.5,  0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0
};

float right_verts[] = {
    0.5, -0.5,  0.5, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.5, -0.5, -0.5, 1.0, 0.0, 1.0, 0.0, 0.0,
    0.5,  0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0,
    0.5,  0.5,  0.5, 0.0, 1.0, 1.0, 0.0, 0.0
};

// The indices is the same for each face
static uint indices[]{
    0,  1,  2,
    2,  3,  0,
};

ResourceManager::ResourceManager()
{
    m_FaceIBuffer = std::make_shared<Dodo::IndexBuffer>(indices, 6);
    Ref<Dodo::Texture> texture = std::make_shared<Dodo::Texture>("res/texture/grass.png", 0, Dodo::TextureSettings(Dodo::TextureFilter::FILTER_MIN_MAG_MIP_NEAREST));
    m_GrassMaterial = std::make_shared<Dodo::Material>(Dodo::Shader::CreateFromPath("block", "res/shader/block.glsl"), texture);

    m_FrontVBuffer = std::make_shared<Dodo::VertexBuffer>(front_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_TopVBuffer = std::make_shared<Dodo::VertexBuffer>(top_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_BackVBuffer = std::make_shared<Dodo::VertexBuffer>(back_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_BottomVBuffer = std::make_shared<Dodo::VertexBuffer>(bottom_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_LeftVBuffer = std::make_shared<Dodo::VertexBuffer>(left_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    m_RightVBuffer = std::make_shared<Dodo::VertexBuffer>(right_verts, 4 * 8 * 4, Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
}