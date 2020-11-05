#include "pch.h"

#include "Rectangle2DComponent.h"
#include "Core/Application/Application.h"

namespace Dodo {

	Rectangle2DComponent::Rectangle2DComponent(const char* path, const Math::Transformation& transformation)
		: m_Transformation(transformation), m_Path(path)
	{}

	Rectangle2DComponent::~Rectangle2DComponent()
	{}
}