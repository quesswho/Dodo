#include "Mesh.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, Ref<Material> material)
        : m_VBuffer(vertexBuffer), m_IBuffer(indexBuffer), m_Material(material)
    {}

    Mesh::~Mesh()
    {
        delete m_VBuffer;
        delete m_IBuffer;
    }

    void Mesh::Draw(RenderAPI& renderAPI) const
    {
        m_Material->Bind(renderAPI);
        m_VBuffer->Bind();
        m_IBuffer->Bind();
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }

    void Mesh::DrawGeometry(RenderAPI& renderAPI) const
    {
        m_VBuffer->Bind();
        m_IBuffer->Bind();
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }

    void Mesh::Draw(Ref<Material> material, RenderAPI& renderAPI) const
    {
        material->Bind(renderAPI);
        m_VBuffer->Bind();
        m_IBuffer->Bind();
        renderAPI.DrawIndices(m_IBuffer->GetCount());
    }
} // namespace Dodo