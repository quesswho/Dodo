#include "MeshFactory.h"
#include "pch.h"

namespace Dodo {
    MeshFactory::MeshFactory() : m_BasicProperties({{"POSITION", 3}, {"TEXCOORD", 2}}), m_RectangleMesh(0) {}

    Mesh* MeshFactory::GetRectangleMesh(Ref<Material> material)
    {
        if (m_RectangleMesh) return m_RectangleMesh;

        float vertices[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
        uint indices[] = {0, 1, 2, 3, 2, 1};
        m_RectangleMesh = new Mesh(new VertexBuffer(vertices, sizeof(vertices), m_BasicProperties),
                                   new IndexBuffer(indices, sizeof(indices) / sizeof(indices[0])), material);

        return m_RectangleMesh;
    }

    Mesh* MeshFactory::CreateCube(Ref<Material> material)
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
        m_CubeMesh = new Mesh(new VertexBuffer(vertices, sizeof(vertices), m_BasicProperties),
                              new IndexBuffer(indices, sizeof(indices) / sizeof(indices[0])), material);

        return m_CubeMesh;
    }
} // namespace Dodo