#include "pch.h"

#include "ModelComponent.h"
#include "Core/Application/Application.h"

namespace Dodo {
	
	ModelComponent::ModelComponent(const char* path, const Math::Transformation& transformation)
		: m_Model(Application::s_Application->m_AssetManager->GetModel(path)), m_Transformation(transformation), m_Path(path)
	{}

	ModelComponent::~ModelComponent()
	{}
}