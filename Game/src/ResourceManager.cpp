#include "ResourceManager.h"

float front_verts[] = {-0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5,  -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
                       0.5,  0.5,  0.5, 1.0, 1.0, 0.0, 0.0, 1.0, -0.5, 0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1.0};

float top_verts[] = {-0.5, 0.5, 0.5,  0.0, 0.0, 0.0, 1.0, 0.0, 0.5,  0.5, 0.5,  1.0, 0.0, 0.0, 1.0, 0.0,
                     0.5,  0.5, -0.5, 1.0, 1.0, 0.0, 1.0, 0.0, -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0};

float back_verts[] = {0.5,  -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -1.0, -0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, -1.0,
                      -0.5, 0.5,  -0.5, 1.0, 1.0, 0.0, 0.0, -1.0, 0.5,  0.5,  -0.5, 0.0, 1.0, 0.0, 0.0, -1.0};

float bottom_verts[] = {-0.5, -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0, 0.5,  -0.5, -0.5, 1.0, 0.0, 0.0, -1.0, 0.0,
                        0.5,  -0.5, 0.5,  1.0, 1.0, 0.0, -1.0, 0.0, -0.5, -0.5, 0.5,  0.0, 1.0, 0.0, -1.0, 0.0};

float left_verts[] = {-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0, -0.5, -0.5, 0.5,  1.0, 0.0, -1.0, 0.0, 0.0,
                      -0.5, 0.5,  0.5,  1.0, 1.0, -1.0, 0.0, 0.0, -0.5, 0.5,  -0.5, 0.0, 1.0, -1.0, 0.0, 0.0};

float right_verts[] = {0.5, -0.5, 0.5,  0.0, 0.0, 1.0, 0.0, 0.0, 0.5, -0.5, -0.5, 1.0, 0.0, 1.0, 0.0, 0.0,
                       0.5, 0.5,  -0.5, 1.0, 1.0, 1.0, 0.0, 0.0, 0.5, 0.5,  0.5,  0.0, 1.0, 1.0, 0.0, 0.0};

ResourceManager::ResourceManager(Dodo::AssetManager& assetManager, Dodo::RenderAPI& renderAPI)
{
    m_Sampler = renderAPI.CreateSampler(Dodo::SamplerProperties(
        Dodo::SamplerFilter::MIN_MAG_MIP_NEAREST, Dodo::SamplerWrapMode::WRAP_CLAMP_TO_EDGE, Dodo::SamplerWrapMode::WRAP_CLAMP_TO_EDGE));

    Dodo::ShaderID id = assetManager.LoadShaderFromPath("res/shader/game/block.slang");
    Dodo::PipelineDesc desc;
    desc.shaderID = id;
    desc.vertexLayout = Dodo::BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TEXINDEX", 1}});

    Dodo::PipelineID pipeId = assetManager.CreatePipeline(desc, renderAPI);
    m_Pipeline = assetManager.GetPipeline(pipeId);

    Dodo::TextureID dirt      = assetManager.LoadTexture("res/texture/blocks/dirt.png");
    Dodo::TextureID grassTop  = assetManager.LoadTexture("res/texture/blocks/grass_top.png");
    Dodo::TextureID grassSide = assetManager.LoadTexture("res/texture/blocks/grass_side.png");
    Dodo::TextureID stone     = assetManager.LoadTexture("res/texture/blocks/stone.png");
    Dodo::TextureID sand      = assetManager.LoadTexture("res/texture/blocks/sand.png");

    RegisterBlock(DIRT,  dirt);
    RegisterBlock(GRASS, grassTop, dirt, grassSide);
    RegisterBlock(STONE, stone);
    RegisterBlock(SAND,  sand);
}

bool ResourceManager::TryFinalize(Dodo::AssetManager& assets, Dodo::RenderAPI& /*renderAPI*/)
{
    if (m_Finalized) return true;

    auto allLoaded = [&](const std::unordered_map<BlockType, Dodo::TextureID>& map) {
        for (auto& [type, id] : map)
            if (assets.GetTextureState(id) != Dodo::AssetState::Loaded) return false;
        return true;
    };

    if (!allLoaded(m_TopTexture)    || !allLoaded(m_BottomTexture) ||
        !allLoaded(m_FrontTexture)  || !allLoaded(m_BackTexture)   ||
        !allLoaded(m_LeftTexture)   || !allLoaded(m_RightTexture))
        return false;

    auto resolveHandles = [&](const std::unordered_map<BlockType, Dodo::TextureID>& texMap,
                               std::unordered_map<BlockType, uint32_t>& handleMap) {
        for (auto& [type, id] : texMap)
            handleMap[type] = assets.GetTexture(id)->GetBindlessHandle();
    };

    resolveHandles(m_TopTexture,    m_TopHandle);
    resolveHandles(m_BottomTexture, m_BottomHandle);
    resolveHandles(m_FrontTexture,  m_FrontHandle);
    resolveHandles(m_BackTexture,   m_BackHandle);
    resolveHandles(m_LeftTexture,   m_LeftHandle);
    resolveHandles(m_RightTexture,  m_RightHandle);

    m_Finalized = true;
    return true;
}

void ResourceManager::RegisterBlock(BlockType type, Dodo::TextureID top, Dodo::TextureID bottom,
                                    Dodo::TextureID front, Dodo::TextureID back, Dodo::TextureID left,
                                    Dodo::TextureID right)
{
    m_TopTexture.emplace(type, top);
    m_BottomTexture.emplace(type, bottom);
    m_FrontTexture.emplace(type, front);
    m_BackTexture.emplace(type, back);
    m_LeftTexture.emplace(type, left);
    m_RightTexture.emplace(type, right);
}

FaceData ResourceManager::GetTopFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_TopHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + top_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + top_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + top_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = top_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = top_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = top_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = top_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = top_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}

FaceData ResourceManager::GetBottomFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_BottomHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + bottom_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + bottom_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + bottom_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = bottom_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = bottom_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = bottom_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = bottom_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = bottom_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}

FaceData ResourceManager::GetFrontFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_FrontHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + front_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + front_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + front_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = front_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = front_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = front_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = front_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = front_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}

FaceData ResourceManager::GetBackFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_BackHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + back_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + back_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + back_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = back_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = back_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = back_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = back_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = back_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}

FaceData ResourceManager::GetLeftFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_LeftHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + left_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + left_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + left_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = left_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = left_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = left_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = left_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = left_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}

FaceData ResourceManager::GetRightFace(BlockType type, BlockPos pos)
{
    float handle = (float)m_RightHandle.at(type);
    FaceData result;
    for (int i = 0; i < 4; i++) {
        result.verts[i * 9]     = pos.x + right_verts[i * 8];
        result.verts[i * 9 + 1] = pos.y + right_verts[i * 8 + 1];
        result.verts[i * 9 + 2] = pos.z + right_verts[i * 8 + 2];
        result.verts[i * 9 + 3] = right_verts[i * 8 + 3];
        result.verts[i * 9 + 4] = right_verts[i * 8 + 4];
        result.verts[i * 9 + 5] = right_verts[i * 8 + 5];
        result.verts[i * 9 + 6] = right_verts[i * 8 + 6];
        result.verts[i * 9 + 7] = right_verts[i * 8 + 7];
        result.verts[i * 9 + 8] = handle;
    }
    return result;
}
