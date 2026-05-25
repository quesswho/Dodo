#pragma once

#include <Dodo.h>

#include "world/Block.h"

class ResourceManager {
  private:
    std::unordered_map<BlockType, Dodo::TextureID> m_TopTexture;
    std::unordered_map<BlockType, Dodo::TextureID> m_BottomTexture;
    std::unordered_map<BlockType, Dodo::TextureID> m_FrontTexture;
    std::unordered_map<BlockType, Dodo::TextureID> m_BackTexture;
    std::unordered_map<BlockType, Dodo::TextureID> m_LeftTexture;
    std::unordered_map<BlockType, Dodo::TextureID> m_RightTexture;

    std::unordered_map<BlockType, uint32_t> m_TopHandle;
    std::unordered_map<BlockType, uint32_t> m_BottomHandle;
    std::unordered_map<BlockType, uint32_t> m_FrontHandle;
    std::unordered_map<BlockType, uint32_t> m_BackHandle;
    std::unordered_map<BlockType, uint32_t> m_LeftHandle;
    std::unordered_map<BlockType, uint32_t> m_RightHandle;

    bool m_Finalized = false;

  public:
    ResourceManager(Dodo::AssetManager& assetManager, Dodo::RenderAPI& renderAPI);

    bool TryFinalize(Dodo::AssetManager& assets, Dodo::RenderAPI& renderAPI);

    Ref<Dodo::IndexBuffer> m_FaceIBuffer;
    Ref<Dodo::Pipeline>    m_Pipeline;
    Ref<Dodo::TextureSampler> m_Sampler;

    FaceData GetTopFace(BlockType type, BlockPos pos);
    FaceData GetBottomFace(BlockType type, BlockPos pos);
    FaceData GetFrontFace(BlockType type, BlockPos pos);
    FaceData GetBackFace(BlockType type, BlockPos pos);
    FaceData GetLeftFace(BlockType type, BlockPos pos);
    FaceData GetRightFace(BlockType type, BlockPos pos);

    inline void RegisterBlock(BlockType type, Dodo::TextureID tex)
    {
        RegisterBlock(type, tex, tex, tex);
    }

    inline void RegisterBlock(BlockType type, Dodo::TextureID top, Dodo::TextureID bottom, Dodo::TextureID side)
    {
        RegisterBlock(type, top, bottom, side, side, side, side);
    }

    void RegisterBlock(BlockType type, Dodo::TextureID top, Dodo::TextureID bottom, Dodo::TextureID front,
                       Dodo::TextureID back, Dodo::TextureID left, Dodo::TextureID right);
};
