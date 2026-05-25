#include "World.h"

World::World(Ref<ResourceManager> resourceManager, Ref<WorldRenderer> worldRenderer, Dodo::RenderAPI& renderAPI)
    : m_WorldRenderer(worldRenderer), m_ResourceManager(resourceManager)
{
    m_WorldGen = std::make_shared<WorldGeneration>(0, resourceManager);
    int radius = 10;
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            ChunkPos pos = ChunkPos(x, y);
            Ref<Chunk> chunk = m_WorldGen->GenerateChunk(pos);
            m_Chunks.emplace(pos, chunk);
        }
    }

    // Update chunks
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            UpdateChunk(ChunkPos(x, y), renderAPI);
        }
    }
}

void World::UpdateChunk(ChunkPos cp, Dodo::RenderAPI& renderAPI)
{
    Ref<Chunk>& chunk = m_Chunks.at(cp);

    auto findChunk = [&](int dx, int dz) -> Chunk* {
        auto it = m_Chunks.find(ChunkPos(cp.x + dx, cp.y + dz));
        return it != m_Chunks.end() ? it->second.get() : nullptr;
    };
    Chunk* cNorth = findChunk(0,  1);
    Chunk* cSouth = findChunk(0, -1);
    Chunk* cEast  = findChunk( 1,  0);
    Chunk* cWest  = findChunk(-1,  0);

    std::vector<FaceData> faces;
    std::vector<uint> indices;
    faces.reserve(4096 * 6);
    indices.reserve(4096 * 36);

    uint i = 0;
    for (int cx = 0; cx < 16; cx++) {
        for (int y = 0; y < 16; y++) {
            for (int cz = 0; cz < 16; cz++) {
                BlockType type = chunk->GetBlockType(cx, y, cz);
                if (type == BlockType::AIR) continue;

                auto getNeighbor = [&](int nx, int ny, int nz) -> BlockType {
                    if (ny < 0 || ny >= 16) return BlockType::AIR;
                    if (nx < 0)   return cWest  ? cWest->GetBlockType(15, ny, nz)  : BlockType::AIR;
                    if (nx >= 16) return cEast  ? cEast->GetBlockType(0,  ny, nz)  : BlockType::AIR;
                    if (nz < 0)   return cSouth ? cSouth->GetBlockType(nx, ny, 15) : BlockType::AIR;
                    if (nz >= 16) return cNorth ? cNorth->GetBlockType(nx, ny, 0)  : BlockType::AIR;
                    return chunk->GetBlockType(nx, ny, nz);
                };

                auto addFace = [&](FaceData face) {
                    faces.push_back(face);
                    indices.push_back(i);
                    indices.push_back(i + 1);
                    indices.push_back(i + 2);
                    indices.push_back(i + 2);
                    indices.push_back(i + 3);
                    indices.push_back(i);
                    i += 4;
                };

                BlockPos bp(cx, y, cz);
                if (getNeighbor(cx, y, cz + 1) == BlockType::AIR) addFace(m_ResourceManager->GetFrontFace(type, bp));
                if (getNeighbor(cx, y, cz - 1) == BlockType::AIR) addFace(m_ResourceManager->GetBackFace(type, bp));
                if (getNeighbor(cx - 1, y, cz) == BlockType::AIR) addFace(m_ResourceManager->GetLeftFace(type, bp));
                if (getNeighbor(cx + 1, y, cz) == BlockType::AIR) addFace(m_ResourceManager->GetRightFace(type, bp));
                if (getNeighbor(cx, y - 1, cz) == BlockType::AIR) addFace(m_ResourceManager->GetBottomFace(type, bp));
                if (getNeighbor(cx, y + 1, cz) == BlockType::AIR) addFace(m_ResourceManager->GetTopFace(type, bp));
            }
        }
    }
    chunk->m_Vertbuffer =
        renderAPI.CreateVertexBuffer((float*)faces.data(), faces.size() * sizeof(FaceData),
                                     Dodo::BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}}));
    chunk->m_Indexbuffer = renderAPI.CreateIndexBuffer(indices.data(), indices.size());
}

BlockType World::GetBlockType(int x, int y, int z)
{
    if (y < 0 || y >= 16) return BlockType::AIR;
    int chunkX = (int)floor((float)x / 16.0f);
    int chunkZ = (int)floor((float)z / 16.0f);
    auto it = m_Chunks.find(ChunkPos(chunkX, chunkZ));
    if (it == m_Chunks.end()) return BlockType::AIR;
    int lx = ((x % 16) + 16) % 16;
    int lz = ((z % 16) + 16) % 16;
    return it->second->m_Blocks[(lx << 8) | (y << 4) | lz];
}
