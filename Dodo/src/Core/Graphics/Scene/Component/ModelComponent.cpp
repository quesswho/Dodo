#include "pch.h"

#include "ModelComponent.h"
#include "Core/Application/Application.h"

namespace Dodo {
	
	ModelComponent::ModelComponent(const char* path, const Math::Transformation& transformation)
		: m_Model(Application::s_Application->m_AssetManager->GetModel(path)), m_Transformation(transformation), m_Path(path)
	{}

	ModelComponent::~ModelComponent()
	{}

	void ModelComponent::Draw(const Math::FreeCamera* camera) const
	{
		m_Model->Bind();
		m_Model->SetUniform("u_Model", m_Transformation.m_Model);
		m_Model->SetUniform("u_Camera", camera->GetCameraMatrix());
		m_Model->SetUniform("u_CameraPos", camera->GetCameraPos());
		m_Model->Draw();
	}

}