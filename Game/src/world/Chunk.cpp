#include "Chunk.h"

Chunk::Chunk(ChunkPos chunkpos) : m_ChunkPos(chunkpos)
{}

Chunk::Chunk(ChunkPos chunkpos, const std::unordered_map<int, std::array<Ref<Block>, 4096>> &blocks)
    : m_ChunkPos(chunkpos), m_Blocks(blocks)
{}

BlockType Chunk::GetBlockType(int x, int y, int z)
{
    if (x >= 0 && x < 16 && y >= 0 && z >= 0 && z < 16)
    {
        auto &subchunk = m_Blocks.find(y / 16);
        if (subchunk != m_Blocks.end())
        {
            if (subchunk->second[(x << 8) + ((y % 16) << 4) + z] != nullptr)
            {
                return subchunk->second[(x << 8) + ((y % 16) << 4) + z]->m_Type;
            }
        }
    }
    return BlockType::AIR;
}

void Chunk::SetBlockType(int x, int y, int z, BlockType type)
{
    if (x >= 0 && x < 16 && y >= 0 && z >= 0 && z < 16)
    {
        auto &subchunk = m_Blocks.find(y / 16);
        if (subchunk != m_Blocks.end())
        {
            subchunk->second[(x << 8) + (y << 4) + z]->m_Type = type;
        } else
        {
            std::array<Ref<Block>, 4096> subchunk;
            subchunk[(x << 8) + ((y % 16) << 4) + z]->m_Type = type;
            m_Blocks.emplace(y / 16, subchunk);
        }
    }
    // Illegal block
}
