#pragma once

#include <Dodo.h>

#include "world/Block.h"

class ResourceManager {
  private:
    std::unordered_map<BlockType, Ref<Block>> m_Blocks;

    // VhunkPos container works for texture coordinates
    std::unordered_map<BlockType, ChunkPos> m_TopTexture;
    std::unordered_map<BlockType, ChunkPos> m_BottomTexture;
    std::unordered_map<BlockType, ChunkPos> m_FrontTexture;
    std::unordered_map<BlockType, ChunkPos> m_BackTexture;
    std::unordered_map<BlockType, ChunkPos> m_LeftTexture;
    std::unordered_map<BlockType, ChunkPos> m_RightTexture;

  public:
    ResourceManager();

    Ref<Dodo::IndexBuffer> m_FaceIBuffer;

    // Texture atlas
    Ref<Dodo::Material> m_TextureAtlas;

    uint m_SizeX;
    uint m_SizeY;

    FaceData GetTopFace(BlockType type, BlockPos pos);
    FaceData GetBottomFace(BlockType type, BlockPos pos);
    FaceData GetFrontFace(BlockType type, BlockPos pos);
    FaceData GetBackFace(BlockType type, BlockPos pos);
    FaceData GetLeftFace(BlockType type, BlockPos pos);
    FaceData GetRightFace(BlockType type, BlockPos pos);

    inline void RegisterBlock(BlockType type, ChunkPos pos) { RegisterBlock(type, pos, pos, pos); }

    inline void RegisterBlock(BlockType type, ChunkPos top, ChunkPos bottom, ChunkPos side)
    {
        RegisterBlock(type, top, bottom, side, side, side, side);
    }

    void RegisterBlock(BlockType type, ChunkPos top, ChunkPos bottom, ChunkPos front, ChunkPos back, ChunkPos left,
                       ChunkPos right);
    Ref<Block> GetBlock(BlockType type);
};
