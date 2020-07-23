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

	void ModelComponent::Move(const Math::Vec3& pos)
	{
		m_Transformation.Move(pos);
	}

	void ModelComponent::Scale(const Math::Vec3& scale)
	{
		m_Transformation.Scale(scale);
	}

	void ModelComponent::Rotate(float radians, const Math::Vec3& axis)
	{
		m_Transformation.Rotate(radians, axis);
	}

	void ModelComponent::Transformation(const Math::Vec3& pos, const Math::Vec3& scale, const Math::Vec3& axis, float radians)
	{
		m_Transformation.Transformate(pos, scale, axis, radians);
	}

	void ModelComponent::Draw() const
	{
		m_Model->Draw();
	}

}