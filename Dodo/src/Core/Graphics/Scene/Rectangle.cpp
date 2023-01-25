#include "pch.h"
#include "Rectangle.h"

#include "Core/Application/Application.h"

namespace Dodo {

	Rectangle::Rectangle(Material* material)
		: m_Mesh(Application::s_Application->m_AssetManager->m_MeshFactory->GetRectangleMesh(material)), m_Material(material)
	{}

	Rectangle::~Rectangle()
	{}

	void Rectangle::Bind() const
	{
		m_Material->Bind();
	}

	void Rectangle::Draw() const
	{
		m_Mesh->Draw();
	}
}