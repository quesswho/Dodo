#include "pch.h"
#include "Rectangle.h"

#include "Core/Application/Application.h"

namespace Dodo {

	Rectangle::Rectangle(Material* material)
		: m_Mesh(Application::s_Application->m_AssetManager->m_MeshFactory->GetRectangleMesh(material))
	{}

	Rectangle::~Rectangle()
	{}

	void Rectangle::Draw() const
	{
		m_Mesh->Draw();
	}

	void Rectangle::Draw(Material* material) const
	{
		m_Mesh->Draw(material);
	}
}