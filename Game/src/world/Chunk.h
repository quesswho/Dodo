#pragma once

#include <Dodo.h>

#include "Block.h"

class Chunk {
  public:
    Chunk(ChunkPos chunkpos);

    inline BlockType GetBlockType(int x, int y, int z) const { return m_Blocks[(x << 8) | (y << 4) | z]; }
    inline void SetBlockType(int x, int y, int z, BlockType t) { m_Blocks[(x << 8) | (y << 4) | z] = t; }

    ChunkPos m_ChunkPos;
    std::array<BlockType, 4096> m_Blocks;
    Ref<Dodo::VertexBuffer> m_Vertbuffer;
    Ref<Dodo::IndexBuffer> m_Indexbuffer;
};
