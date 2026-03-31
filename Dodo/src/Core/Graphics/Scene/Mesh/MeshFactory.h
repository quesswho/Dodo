#pragma once

#include "Mesh.h"

namespace Dodo {
    struct TerrainConfig {
        uint32_t resolution = 256;  // Vertices per side (e.g., 256 = 256x256 grid)
        float size = 100.0f;       // World-space size
        float heightScale = 50.0f; // Multiplier for height values
        uint32_t seed = 0;
        uint32_t octaves = 6;
        float persistence = 0.5f;
        float frequency = 0.005f;
    };

    class MeshFactory {
      public:
        MeshFactory();

        Ref<Mesh> GetRectangleMesh(Ref<Material> material);
        Ref<Mesh> CreateCube(Ref<Material> material);

        Ref<Mesh> CreateTerrain(const TerrainConfig& config, Ref<Material> material);

      private:
        const BufferProperties m_BasicProperties;
        Ref<Mesh> m_RectangleMesh = nullptr;
        Ref<Mesh> m_CubeMesh = nullptr;
    };
} // namespace Dodo