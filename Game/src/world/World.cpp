#include "World.h"

World::World(Ref<ResourceManager> resourceManager, Ref<WorldRenderer> worldRenderer)
	: m_WorldRenderer(worldRenderer)
{
	m_WorldGen = std::make_shared<WorldGeneration>(0, resourceManager);
	for (int x = -10; x <= 40; x++) {
		for (int y = -10; y <= 40; y++) {
			ChunkPos pos = ChunkPos(x, y);
			Ref<Chunk> chunk = m_WorldGen->GenerateChunk(pos);
			m_Chunks.emplace(pos, chunk);
		}
	}

    // Update chunks
    for (int x = -10; x <= 40; x++) {
        for (int y = -10; y <= 40; y++) {
            UpdateChunk(ChunkPos(x, y));
        }
    }
}

void World::UpdateChunk(ChunkPos chunkpos) {
    Ref<Chunk> chunk = m_Chunks.at(chunkpos);
    std::vector<FaceData> faces;
    std::vector<uint> indices;
    uint i = 0;
    for (int cx = 0; cx < 16; cx++) {
        for (int y = 0; y < 16; y++) {
            for (int cz = 0; cz < 16; cz++) {
                BlockType type = chunk->GetBlockType(cx, y, cz);
                int x = (chunkpos.x << 4) + cx;
                int z = (chunkpos.y << 4) + cz;
                if (type != BlockType::AIR) {
                    if (GetBlockType(x, y, z + 1) == BlockType::AIR) { // front
                        faces.push_back(m_ResourceManager->GetFrontFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (GetBlockType(x, y, z - 1) == BlockType::AIR) { // back
                        faces.push_back(m_ResourceManager->GetBackFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }

                    if (GetBlockType(x - 1, y, z) == BlockType::AIR) { // right
                        faces.push_back(m_ResourceManager->GetLeftFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (GetBlockType(x + 1, y, z) == BlockType::AIR) { // left
                        faces.push_back(m_ResourceManager->GetRightFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }

                    if (GetBlockType(x, y - 1, z) == BlockType::AIR) { // bottom
                        faces.push_back(m_ResourceManager->GetBottomFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                    if (GetBlockType(x, y + 1, z) == BlockType::AIR) { // top
                        faces.push_back(m_ResourceManager->GetTopFace(type, BlockPos(cx, y, cz)));
                        indices.insert(indices.end(), { i, i + 1, i + 2, i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
            }
        }
    }
    chunk->m_Vertbuffer = std::make_shared<Dodo::VertexBuffer>((float*)&faces[0], faces.size() * sizeof(FaceData), Dodo::BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3} }));
    chunk->m_Indexbuffer = std::make_shared<Dodo::IndexBuffer>(indices.data(), indices.size());
}

BlockType World::GetBlockType(int x, int y, int z) {
    auto temp = m_Chunks.find(ChunkPos(floor(x / 16.0f), floor(z / 16.0f)));
    if (temp != m_Chunks.end()) {
        x = (x % 16 + 16) % 16;
        z = (z % 16 + 16) % 16;
        if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16) {
            return temp->second->m_Blocks[(x << 8) + (y << 4) + z]->m_Type;
        }
    }
    return BlockType::AIR;
}