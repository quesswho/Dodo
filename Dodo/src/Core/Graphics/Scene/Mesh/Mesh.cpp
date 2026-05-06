#include "Mesh.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Mesh::Mesh(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<Material> material)
        : m_VBuffer(vertexBuffer), m_IBuffer(indexBuffer), m_Material(material)
    {}

    Mesh::~Mesh() {}

    void Mesh::Draw(RenderAPI& renderAPI) const
    {
        m_Material->Bind(renderAPI);
        renderAPI.BindVertexBuffer(m_VBuffer);
        renderAPI.BindIndexBuffer(m_IBuffer);
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }

    void Mesh::DrawGeometry(RenderAPI& renderAPI) const
    {
        renderAPI.BindVertexBuffer(m_VBuffer);
        renderAPI.BindIndexBuffer(m_IBuffer);
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }

    void Mesh::DrawGeometryInstanced(RenderAPI& renderAPI, uint instances) const
    {
        renderAPI.BindVertexBuffer(m_VBuffer);
        renderAPI.BindIndexBuffer(m_IBuffer);
        renderAPI.DrawIndicesInstanced(m_IBuffer->GetCount(), instances);
    }

    void Mesh::Draw(Ref<Material> material, RenderAPI& renderAPI) const
    {
        material->Bind(renderAPI);
        renderAPI.BindVertexBuffer(m_VBuffer);
        renderAPI.BindIndexBuffer(m_IBuffer);
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }
} // namespace Dodo
