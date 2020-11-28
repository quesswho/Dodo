#include "pch.h"
#include "MeshFactory.h"

namespace Dodo
{
	MeshFactory::MeshFactory()
		: m_BasicProperties( { {"POSITION", 3 }, { "TEXCOORD", 2 }}), m_RectangleMesh(0)
	{}

	Mesh* MeshFactory::GetRectangleMesh() {
		if (!m_RectangleMesh)
		{

			float vertices[] = {
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f
			};
			uint indices[] = {
				0,1,2,3,2,1
			};
			return new Mesh(new VertexBuffer(vertices, sizeof(vertices), m_BasicProperties), new IndexBuffer(indices, _countof(indices)));
		}
		else
			return m_RectangleMesh;
		
	}
}