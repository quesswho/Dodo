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

    RegisterBlock(AIR);
    RegisterBlock(DIRT);
    RegisterBlock(GRASS);
}

void ResourceManager::RegisterBlock(BlockType type) {
    m_Blocks.emplace(type, std::make_shared<Block>(type));
}

Ref<Block> ResourceManager::GetBlock(BlockType type) {
    return m_Blocks.find(type)->second;
}

FaceData ResourceManager::GetFrontFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 2;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i*8] = pos.x + front_verts[i * 8];
        result.verts[i*8+1] = pos.y + front_verts[i * 8 + 1];
        result.verts[i*8+2] = pos.z + front_verts[i * 8 + 2];
        result.verts[i*8+3] = (u + front_verts[i*8+3]) / 16.0f;
        result.verts[i*8+4] = 1.0f - (v + front_verts[i * 8 + 4]) / 16.0f;
        result.verts[i*8+5] = 0.0f;
        result.verts[i*8+6] = 0.0f;
        result.verts[i*8+7] = 1.0f;
    }
    return result;
}

FaceData ResourceManager::GetBackFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 2;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + back_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + back_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + back_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (u + back_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (v + back_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = -1.0f;
    }
    return result;
}

FaceData ResourceManager::GetTopFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 1;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + top_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + top_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + top_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (u + top_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (v + top_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = 1.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetBottomFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 1;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + bottom_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + bottom_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + bottom_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (u + bottom_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (v + bottom_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 0.0f;
        result.verts[i * 8 + 6] = -1.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetLeftFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 2;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + left_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + left_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + left_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (u + left_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (v + left_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = -1.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}

FaceData ResourceManager::GetRightFace(BlockType type, BlockPos pos) {
    int u = 0;
    switch (type) {
    case DIRT:
        u = 0;
        break;
    case GRASS:
        u = 2;
        break;
    default:
        u = 0;
        break;
    }
    int v = 0;

    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 8] = pos.x + right_verts[i * 8];
        result.verts[i * 8 + 1] = pos.y + right_verts[i * 8 + 1];
        result.verts[i * 8 + 2] = pos.z + right_verts[i * 8 + 2];
        result.verts[i * 8 + 3] = (u + right_verts[i * 8 + 3]) / 16.0f;
        result.verts[i * 8 + 4] = 1.0f - (v + right_verts[i * 8 + 4]) / 16.0f;
        result.verts[i * 8 + 5] = 1.0f;
        result.verts[i * 8 + 6] = 0.0f;
        result.verts[i * 8 + 7] = 0.0f;
    }
    return result;
}