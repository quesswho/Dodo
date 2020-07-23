#include "pch.h"

#include "ModelComponent.h"

namespace Dodo {
	
	ModelComponent::ModelComponent(const char* path, const Math::Transformation& transformation)
		: m_Model(new Model(path)), m_Transformation(transformation), m_Path(path)
	{}

	ModelComponent::~ModelComponent()
	{
		delete m_Model;
	}

	void ModelComponent::Transformation(const Math::Vec3& pos, const Math::Vec3& scale, const Math::Vec3& axis, float radians)
	{
		m_Transformation.Transformate(pos, scale, axis, radians);
	}

	void ModelComponent::Draw(const Math::Mat4& camera) const
	{
		m_Model->Bind();
		m_Model->SetUniform("u_Model", m_Transformation.m_Model);
		m_Model->SetUniform("u_Camera", camera);
		m_Model->Draw();
	}

}