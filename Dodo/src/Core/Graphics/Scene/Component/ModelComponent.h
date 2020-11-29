#pragma once

#include "Components.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"
#include "Core/Math/MathFunc.h"

namespace Dodo {

	class ModelComponent {
	private:
		Model* m_Model;
	public:
		Math::Transformation m_Transformation;

		ModelComponent(const char* path)
			: ModelComponent(path, Math::Transformation())
		{}

		ModelComponent(const char* path, const Math::Transformation& transformation);

		~ModelComponent();

		template<typename T>
		void Draw(const T* camera) const
		{
			m_Model->Bind();
			m_Model->SetUniform("u_Model", m_Transformation.m_Model);
			m_Model->SetUniform("u_Camera", camera->GetCameraMatrix());
			m_Model->SetUniform("u_CameraPos", camera->GetCameraPos());
			m_Model->Draw();
		}

		static inline bool IsDrawable() { return true; }
		static inline const std::string& GetName() { return std::string("ModelComponent"); }
		static constexpr ComponentFlag GetFlagType() { return ComponentFlag::ComponentFlag_ModelComponent; }
		static constexpr int GetIndex() { return Math::floorlog2(GetFlagType()); }

		std::string m_Path;
	};
}