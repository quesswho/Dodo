#include "MeshFactory.h"
#include "pch.h"

#include "Core/Math/Random/Noise.h"

namespace Dodo {
    MeshFactory::MeshFactory() : m_BasicProperties({{"POSITION", 3}, {"TEXCOORD", 2}}) {}

    Ref<Mesh> MeshFactory::GetRectangleMesh(Ref<Material> material, RenderAPI& renderAPI)
    {
        if (m_RectangleMesh) return m_RectangleMesh;

        float vertices[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
        uint indices[] = {0, 1, 2, 3, 2, 1};
        m_RectangleMesh = std::make_shared<Mesh>(
            renderAPI.CreateVertexBuffer(vertices, sizeof(vertices), m_BasicProperties),
            renderAPI.CreateIndexBuffer(indices, sizeof(indices) / sizeof(indices[0])), material);

        return m_RectangleMesh;
    }

    Ref<Mesh> MeshFactory::CreateCube(Ref<Material> material, RenderAPI& renderAPI)
    {
        if (m_CubeMesh) return m_CubeMesh;

        // We need to duplicate vertices for each face because of different UVs
        float vertices[] = {
            // Front face
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  0
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //  1
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //  2
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f, //  3

            // Back face
            0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //  4
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, //  5
            1.0f, 0.0f, 1.0f, 0.0f, 0.0f, //  6
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, //  7

            // Left face
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //  8
            0.0f, 1.0f, 1.0f, 0.0f, 1.0f, //  9
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 10
            0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 11

            // Right face
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 12
            1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 13
            1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 14
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // 15

            // Top face
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // 16
            0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 17
            1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 18
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // 19

            // Bottom face
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 20
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 21
            1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 22
            1.0f, 0.0f, 0.0f, 1.0f, 1.0f  // 23
        };

        uint indices[] = {// Front
                          0, 1, 2, 2, 1, 3,
                          // Back
                          4, 6, 5, 6, 7, 5,
                          // Left
                          8, 9, 10, 10, 9, 11,
                          // Right
                          12, 13, 14, 14, 13, 15,
                          // Top
                          16, 17, 18, 18, 17, 19,
                          // Bottom
                          20, 21, 22, 22, 21, 23};
        m_CubeMesh = std::make_shared<Mesh>(renderAPI.CreateVertexBuffer(vertices, sizeof(vertices), m_BasicProperties),
                                            renderAPI.CreateIndexBuffer(indices, sizeof(indices) / sizeof(indices[0])),
                                            material);

        return m_CubeMesh;
    }

    Ref<Mesh> MeshFactory::CreateTerrain(const TerrainConfig& config, Ref<Material> material, RenderAPI& renderAPI)
    {
        const uint32_t res = config.resolution;
        const float step = config.size / (res - 1);
        const float halfSize = config.size * 0.5f;

        // Calculate vertex count (POSITION + NORMAL + TEXCOORD = 3 + 3 + 2 = 8 floats per vertex)
        std::vector<float> vertices;
        vertices.reserve(res * res * 8);

        std::vector<uint32_t> indices;
        indices.reserve((res - 1) * (res - 1) * 6);

        // Temporary storage for positions to calculate normals
        std::vector<Math::Vec3> positions(res * res);

        // Generate vertex positions
        for (uint32_t z = 0; z < res; ++z) {
            for (uint32_t x = 0; x < res; ++x) {
                float worldX = x * step - halfSize;
                float worldZ = z * step - halfSize;
                float height =
                    Math::Noise::SumSimplex(worldX, worldZ, config.octaves, config.persistence, config.frequency) *
                    config.heightScale;

                uint32_t idx = z * res + x;
                positions[idx] = {worldX, height, worldZ};
            }
        }

        // Generate vertices with normals
        BufferProperties terrainProps = {{"POSITION", 3}, {"NORMAL", 3}, {"TEXCOORD", 2}};

        for (uint32_t z = 0; z < res; ++z) {
            for (uint32_t x = 0; x < res; ++x) {
                uint32_t idx = z * res + x;
                const Math::Vec3& pos = positions[idx];

                // Calculate normal using finite differences
                Math::Vec3 normal(0.0f, 1.0f, 0.0f);

                if (x > 0 && x < res - 1 && z > 0 && z < res - 1) {
                    Math::Vec3 left = positions[idx - 1];
                    Math::Vec3 right = positions[idx + 1];
                    Math::Vec3 down = positions[idx - res];
                    Math::Vec3 up = positions[idx + res];

                    Math::Vec3 dx = right - left;
                    Math::Vec3 dz = up - down;

                    normal = Math::Vec3::Normalize(Math::Vec3::Cross(dz, dx));
                }

                // Position
                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);

                // Normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                // TexCoord
                vertices.push_back((float)x / (res - 1));
                vertices.push_back((float)z / (res - 1));
            }
        }

        // Generate indices
        for (uint32_t z = 0; z < res - 1; ++z) {
            for (uint32_t x = 0; x < res - 1; ++x) {
                uint32_t topLeft = z * res + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = (z + 1) * res + x;
                uint32_t bottomRight = bottomLeft + 1;

                // Triangle 1
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Triangle 2
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        return std::make_shared<Mesh>(
            renderAPI.CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(float), terrainProps),
            renderAPI.CreateIndexBuffer(indices.data(), indices.size()), material);
    }

    Ref<VertexBuffer> MeshFactory::GetScreenQuadBuffer(RenderAPI& renderAPI)
    {
        if (m_ScreenQuadBuffer) return m_ScreenQuadBuffer;

#ifdef DD_OPENGL
        // OpenGL framebuffer textures have V=0 at the bottom, so flip V to display correctly
        float vertices[] = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f
        };
#else
        // Canonical convention (Vulkan): V=0 at the top
        float vertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 0.0f
        };
#endif
        m_ScreenQuadBuffer = renderAPI.CreateVertexBuffer(
            vertices, sizeof(vertices),
            BufferProperties({{"POSITION", 2}, {"TEXCOORD", 2}}));
        return m_ScreenQuadBuffer;
    }
} // namespace Dodo