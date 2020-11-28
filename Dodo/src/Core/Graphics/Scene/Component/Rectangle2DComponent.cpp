#include "pch.h"

#include "Rectangle2DComponent.h"
#include "Core/Application/Application.h"

namespace Dodo {

	Rectangle2DComponent::Rectangle2DComponent(const char* path, const Math::Transformation& transformation)
		: m_Transformation(transformation), m_Path(path), m_Rectangle(new Rectangle(Application::s_Application->m_AssetManager->GetMaterial(path)))
	{}

	Rectangle2DComponent::~Rectangle2DComponent()
	{
		delete m_Rectangle;
	}
}