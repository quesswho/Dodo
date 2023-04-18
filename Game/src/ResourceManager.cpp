#include "ResourceManager.h"

float front_verts[] = {
    -0.5, -0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0, 0.0, 1.0,
     0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
    -0.5,  0.5,  0.5, 0.0, 0.0, 0.0, 0.0, 1.0
};

float top_verts[] = {
    -0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 0.0, 0.0, 0.0, 1.0, 0.0
};


float back_verts[] = {
     0.5, -0.5, -0.5, 0.0, 1.0, 0.0, 0.0, -1.0,
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0, 0.0, -1.0,
    -0.5,  0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0,
     0.5,  0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0
};


float bottom_verts[] = {
    -0.5, -0.5, -0.5, 0.0, 1.0, 0.0, -1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 1.0, 0.0, -1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 0.0, 0.0, -1.0, 0.0,
    -0.5, -0.5,  0.5, 0.0, 0.0, 0.0, -1.0, 0.0
};

float left_verts[] = {
    -0.5, -0.5, -0.5, 0.0, 1.0, -1.0, 0.0, 0.0,
    -0.5, -0.5,  0.5, 1.0, 1.0, -1.0, 0.0, 0.0,
    -0.5,  0.5,  0.5, 1.0, 0.0, -1.0, 0.0, 0.0,
    -0.5,  0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0
};

float right_verts[] = {
    0.5, -0.5,  0.5, 0.0, 1.0, 1.0, 0.0, 0.0,
    0.5, -0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0,
    0.5,  0.5, -0.5, 1.0, 0.0, 1.0, 0.0, 0.0,
    0.5,  0.5,  0.5, 0.0, 0.0, 1.0, 0.0, 0.0
};

ResourceManager::ResourceManager()
{
    Ref<Dodo::Texture> atlas = std::make_shared<Dodo::Texture>("res/texture/blocks.png", 0, 
        Dodo::TextureSettings(Dodo::TextureFilter::FILTER_MIN_MAG_MIP_NEAREST, Dodo::TextureWrapMode::WRAP_REPEAT, Dodo::TextureWrapMode::WRAP_REPEAT));

    Ref<Dodo::Shader> shader = Dodo::Shader::CreateFromPath("block", "res/shader/block.glsl");

    m_TextureAtlas = std::make_shared<Dodo::Material>(shader, atlas);

    RegisterBlock(AIR, ChunkPos(0,0));
    RegisterBlock(DIRT, ChunkPos(0,0));
    RegisterBlock(GRASS, ChunkPos(1, 0), ChunkPos(0, 0), ChunkPos(2, 0));
    RegisterBlock(STONE, ChunkPos(3, 0));
    RegisterBlock(SAND, ChunkPos(4, 0));
}

void ResourceManager::RegisterBlock(BlockType type, ChunkPos top, ChunkPos bottom, ChunkPos front, ChunkPos back, ChunkPos left, ChunkPos right) {
    m_Blocks.emplace(type, std::make_shared<Block>(type));
    m_TopTexture.emplace(type, top);
    m_BottomTexture.emplace(type, bottom);
    m_FrontTexture.emplace(type, front);
    m_BackTexture.emplace(type, back);
    m_LeftTexture.emplace(type, left);
    m_RightTexture.emplace(type, right);
}

Ref<Block> ResourceManager::GetBlock(BlockType type) {
    return m_Blocks.find(type)->second;
}

FaceData ResourceManager::GetTopFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_TopTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + top_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + top_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + top_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + top_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + top_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = 1.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetBottomFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_BottomTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + bottom_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + bottom_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + bottom_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = -1.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetFrontFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_FrontTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i*8] = pos.x + front_verts[i * 8];
        result.verts[i*8+1] = pos.y + front_verts[i * 8 + 1];
        result.verts[i*8+2] = pos.z + front_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i*8+5] = 0.0f;
        result.verts[i*8+6] = 0.0f;
        result.verts[i*8+7] = 1.0f;
    }
    return result;
}

FaceData ResourceManager::GetBackFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_BackTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + back_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + back_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + back_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = -1.0f;
    }
    return result;
}


FaceData ResourceManager::GetLeftFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_LeftTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + left_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + left_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + left_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = -1.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetRightFace(BlockType type, BlockPos pos) {
    ChunkPos texcoord = m_RightTexture.find(type)->second;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + right_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + right_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + right_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (texcoord.x + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (texcoord.y + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 1.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}