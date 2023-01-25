#include "pch.h"
#include "Mesh.h"

#include "Core/Application/Application.h"

namespace Dodo {

	Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, Material* material)
		: m_VBuffer(vertexBuffer), m_IBuffer(indexBuffer), m_Material(material)
	{}

	Mesh::~Mesh()
	{
		delete m_VBuffer;
		delete m_IBuffer;
	}

	void Mesh::Draw() const
	{
		m_Material->Bind();
		m_VBuffer->Bind();
		m_IBuffer->Bind();
		Application::s_Application->m_RenderAPI->DrawIndices(m_IBuffer->GetCount());
	}
}