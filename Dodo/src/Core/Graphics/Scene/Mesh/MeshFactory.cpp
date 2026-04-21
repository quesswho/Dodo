#include "MeshFactory.h"
#include "pch.h"

#include "Core/Math/Random/Noise.h"

namespace Dodo {
    MeshFactory::MeshFactory() : m_BasicProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 4}}) {}

    Ref<Mesh> MeshFactory::GetRectangleMesh(Ref<Material> material, RenderAPI& renderAPI)
    {
        if (m_RectangleMesh) return m_RectangleMesh;

        // XY plane facing -Z. Format per vertex: pos(3) uv(2) normal(3) tangent(4)
        float vertices[] = {
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        };
        uint indices[] = {0, 1, 2, 3, 2, 1};
        m_RectangleMesh = std::make_shared<Mesh>(
            renderAPI.CreateVertexBuffer(vertices, sizeof(vertices), m_BasicProperties),
            renderAPI.CreateIndexBuffer(indices, sizeof(indices) / sizeof(indices[0])), material);

        return m_RectangleMesh;
    }

    Ref<Mesh> MeshFactory::CreateCube(Ref<Material> material, RenderAPI& renderAPI)
    {
        if (m_CubeMesh) return m_CubeMesh;

        // We need to duplicate vertices for each face because of different UVs.
        // Format per vertex: pos(3) uv(2) normal(3) tangent(4)
#ifdef DD_API_OPENGL
        // OpenGL: V=0 at bottom of texture image
        float vertices[] = {
            // Front face (normal: 0,0,-1  tangent: 1,0,0,1)
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  0
            0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  1
            1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  2
            1.0f, 1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  3

            // Back face (normal: 0,0,1  tangent: -1,0,0,1)
            0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  4
            0.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  5
            1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  6
            1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  7

            // Left face (normal: -1,0,0  tangent: 0,0,-1,1)
            0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, //  8
            0.0f, 1.0f, 1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, //  9
            0.0f, 0.0f, 0.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, // 10
            0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, // 11

            // Right face (normal: 1,0,0  tangent: 0,0,1,1)
            1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 12
            1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 13
            1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 14
            1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 15

            // Top face (normal: 0,1,0  tangent: 1,0,0,1)
            0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 16
            0.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 17
            1.0f, 1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 18
            1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 19

            // Bottom face (normal: 0,-1,0  tangent: 1,0,0,1)
            0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 20
            0.0f, 0.0f, 0.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 21
            1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 22
            1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f  // 23
        };
#else
        // Vulkan: V=0 at top of texture image, so V is flipped relative to OpenGL
        float vertices[] = {
            // Front face (normal: 0,0,-1  tangent: 1,0,0,1)
            0.0f, 0.0f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  0
            0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  1
            1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  2
            1.0f, 1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //  3

            // Back face (normal: 0,0,1  tangent: -1,0,0,1)
            0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  4
            0.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  5
            1.0f, 0.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  6
            1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, //  7

            // Left face (normal: -1,0,0  tangent: 0,0,-1,1)
            0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, //  8
            0.0f, 1.0f, 1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, //  9
            0.0f, 0.0f, 0.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, // 10
            0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, // 11

            // Right face (normal: 1,0,0  tangent: 0,0,1,1)
            1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 12
            1.0f, 1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 13
            1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 14
            1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // 15

            // Top face (normal: 0,1,0  tangent: 1,0,0,1)
            0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 16
            0.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 17
            1.0f, 1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 18
            1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 19

            // Bottom face (normal: 0,-1,0  tangent: 1,0,0,1)
            0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 20
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 21
            1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 22
            1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f  // 23
        };
#endif

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

        // POSITION(3) + TEXCOORD(2) + NORMAL(3) + TANGENT(4) = 12 floats per vertex
        std::vector<float> vertices;
        vertices.reserve(res * res * 12);

        std::vector<uint32_t> indices;
        indices.reserve((res - 1) * (res - 1) * 6);

        std::vector<Math::Vec3> positions(res * res);

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

        BufferProperties terrainProps = {{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 4}};

        for (uint32_t z = 0; z < res; ++z) {
            for (uint32_t x = 0; x < res; ++x) {
                uint32_t idx = z * res + x;
                const Math::Vec3& pos = positions[idx];

                Math::Vec3 normal(0.0f, 1.0f, 0.0f);

                if (x > 0 && x < res - 1 && z > 0 && z < res - 1) {
                    Math::Vec3 dx = positions[idx + 1] - positions[idx - 1];
                    Math::Vec3 dz = positions[idx + res] - positions[idx - res];
                    normal = Math::Vec3::Normalize(Math::Vec3::Cross(dz, dx));
                }

                // Tangent: finite difference in X (UV.u direction), projected onto the surface
                Math::Vec3 dx;
                if (x == 0)
                    dx = positions[idx + 1] - positions[idx];
                else if (x == res - 1)
                    dx = positions[idx] - positions[idx - 1];
                else
                    dx = positions[idx + 1] - positions[idx - 1];

                float dot = dx.Dot(normal);
                Math::Vec3 tangent = Math::Vec3::Normalize({dx.x - dot * normal.x, dx.y - dot * normal.y, dx.z - dot * normal.z});

                // Position
                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);

                // TexCoord
                vertices.push_back((float)x / (res - 1));
                vertices.push_back((float)z / (res - 1));

                // Normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                // Tangent
                vertices.push_back(tangent.x);
                vertices.push_back(tangent.y);
                vertices.push_back(tangent.z);
                vertices.push_back(1.0f);
            }
        }

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

#ifdef DD_API_OPENGL
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
