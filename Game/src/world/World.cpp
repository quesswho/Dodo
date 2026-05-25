#include "World.h"

World::World(Ref<ResourceManager> resourceManager, Ref<WorldRenderer> worldRenderer, Dodo::RenderAPI& /*renderAPI*/)
    : m_WorldRenderer(worldRenderer), m_ResourceManager(resourceManager)
{
    m_WorldGen = std::make_shared<WorldGeneration>(0, resourceManager);
    int radius = 10;
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            for (int z = 0; z < VERTICAL_CHUNKS; z++) {
                ChunkPos pos(x, y, z);
                m_Chunks.emplace(pos, m_WorldGen->GenerateChunk(pos));
            }
        }
    }

}

void World::BuildMeshes(Dodo::RenderAPI& renderAPI)
{
    for (auto& [pos, chunk] : m_Chunks)
        UpdateChunk(pos, renderAPI);
}

void World::UpdateChunk(ChunkPos cp, Dodo::RenderAPI& renderAPI)
{
    Ref<Chunk>& chunk = m_Chunks.at(cp);

    auto findChunk = [&](int dx, int dy, int dz) -> Chunk* {
        auto it = m_Chunks.find(ChunkPos(cp.x + dx, cp.y + dy, cp.z + dz));
        return it != m_Chunks.end() ? it->second.get() : nullptr;
    };
    Chunk* cNorth = findChunk(0,  1,  0);
    Chunk* cSouth = findChunk(0, -1,  0);
    Chunk* cEast  = findChunk( 1,  0,  0);
    Chunk* cWest  = findChunk(-1,  0,  0);
    Chunk* cAbove = findChunk(0,   0,  1);
    Chunk* cBelow = findChunk(0,   0, -1);

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
                    if (ny < 0)   return cBelow ? cBelow->GetBlockType(nx, 15, nz) : BlockType::AIR;
                    if (ny >= 16) return cAbove ? cAbove->GetBlockType(nx,  0, nz) : BlockType::AIR;
                    if (nx < 0)   return cWest  ? cWest->GetBlockType(15, ny, nz)  : BlockType::AIR;
                    if (nx >= 16) return cEast  ? cEast->GetBlockType(0,  ny, nz)  : BlockType::AIR;
                    if (nz < 0)   return cSouth ? cSouth->GetBlockType(nx, ny, 15) : BlockType::AIR;
                    if (nz >= 16) return cNorth ? cNorth->GetBlockType(nx, ny,  0) : BlockType::AIR;
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
    if (faces.empty()) {
        chunk->m_Vertbuffer = nullptr;
        chunk->m_Indexbuffer = nullptr;
        return;
    }
    chunk->m_Vertbuffer =
        renderAPI.CreateVertexBuffer((float*)faces.data(), faces.size() * sizeof(FaceData),
                                     Dodo::BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TEXINDEX", 1}}));
    chunk->m_Indexbuffer = renderAPI.CreateIndexBuffer(indices.data(), indices.size());
}

BlockType World::GetBlockType(int x, int y, int z)
{
    if (y < 0) return BlockType::AIR;
    int cx = (int)floor((float)x / 16.0f);
    int cy = (int)floor((float)z / 16.0f);   // world Z (depth) -> ChunkPos.y
    int cz = (int)floor((float)y / 16.0f);   // world Y (height) -> ChunkPos.z
    auto it = m_Chunks.find(ChunkPos(cx, cy, cz));
    if (it == m_Chunks.end()) return BlockType::AIR;
    int lx = ((x % 16) + 16) % 16;
    int ly = ((y % 16) + 16) % 16;
    int lz = ((z % 16) + 16) % 16;
    return it->second->GetBlockType(lx, ly, lz);
}
