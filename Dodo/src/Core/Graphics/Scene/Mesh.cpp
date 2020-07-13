#include "pch.h"
#include "Mesh.h"

namespace Dodo {

	Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer)
		: m_VBuffer(vertexBuffer), m_IBuffer(indexBuffer)
	{

	}

	Mesh::~Mesh()
	{
		delete m_VBuffer;
		delete m_IBuffer;
	}

	void Mesh::Draw() const
	{
		m_VBuffer->Bind();
		m_IBuffer->Bind();
	}
}