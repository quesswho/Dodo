#include "World.h"

float cube_vertices[] = {
    // front
     -0.5, -0.5, 0.5, 0.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0,
    -0.5,  0.5,  0.5, 0.0, 1.0,
    // top
    -0.5,  0.5,  0.5, 0.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0,
    -0.5,  0.5, -0.5, 0.0, 1.0,
    // back
     0.5, -0.5, -0.5, 0.0, 0.0,
    -0.5, -0.5, -0.5, 1.0, 0.0,
    -0.5,  0.5, -0.5, 1.0, 1.0,
     0.5,  0.5, -0.5, 0.0, 1.0,
     // bottom
     -0.5, -0.5, -0.5, 0.0, 0.0,
      0.5, -0.5, -0.5, 1.0, 0.0,
      0.5, -0.5,  0.5, 1.0, 1.0,
     -0.5, -0.5,  0.5, 0.0, 1.0,
     // left
     -0.5, -0.5, -0.5, 0.0, 0.0,
     -0.5, -0.5,  0.5, 1.0, 0.0,
     -0.5,  0.5,  0.5, 1.0, 1.0,
     -0.5,  0.5, -0.5, 0.0, 1.0,
     // right
      0.5, -0.5,  0.5, 0.0, 0.0,
      0.5, -0.5, -0.5, 1.0, 0.0,
      0.5,  0.5, -0.5, 1.0, 1.0,
      0.5,  0.5,  0.5, 0.0, 1.0
};

World::World(Dodo::Math::FreeCamera* camera) 
    : m_Camera(camera)
{  
    int count = 36;
    uint indices[]{
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
    

	Ref<Dodo::Texture> texture = std::make_shared<Dodo::Texture>("res/texture/grass.png", 0, Dodo::TextureSettings(Dodo::TextureFilter::FILTER_MIN_MAG_MIP_NEAREST));
    Ref<Dodo::Material> material = std::make_shared<Dodo::Material>(Dodo::Shader::CreateFromPath("block", "res/shader/block.glsl"), texture);
    Ref<Dodo::Mesh> mesh = std::make_shared<Dodo::Mesh>(new Dodo::VertexBuffer(cube_vertices, 24 * 4 * 5,
        Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }})),
        new Dodo::IndexBuffer(indices, count), material);

	for (int i = 0; i < 1024; i++) {
		Ref<Block> block = std::make_shared<Block>(mesh);
		m_Blocks[i] = block;
	}
}

void World::Draw() {
    for (int i = 0; i < 1024; i++) {
        m_Blocks[i]->m_Mesh->SetUniform("u_Camera", m_Camera->GetCameraMatrix());
        m_Blocks[i]->m_Mesh->SetUniform("u_Model", Dodo::Math::Mat4::Translate(Dodo::Math::Vec3(-16 + i / 32, 0.0f, -16 + i % 32)));
        m_Blocks[i]->m_Mesh->Draw();
    }
        
}