#pragma once

#include "Components.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"
#include "Core/Math/MathFunc.h"
#include "Core/Graphics/Scene/Light.h"

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
		void Draw(const T* camera, const LightSystem& lights) const
		{
			m_Model->SetUniform("u_LightCamera", lights.m_Directional.m_LightCamera);
			m_Model->SetUniform("u_LightDir", lights.m_Directional.m_Direction);
			m_Model->SetUniform("u_Model", m_Transformation.m_Model);
			m_Model->SetUniform("u_Camera", camera->GetCameraMatrix());
			m_Model->SetUniform("u_CameraPos", camera->GetCameraPos());
			m_Model->Draw();
		}

		void Draw(Ref<Material> material) const
		{
			material->SetUniform("u_Model", m_Transformation.m_Model);
			m_Model->Draw(material);
		}

		static inline bool IsDrawable() { return true; }
		static inline const std::string& GetName() { return std::string("ModelComponent"); }
		static constexpr ComponentFlag GetFlagType() { return ComponentFlag::ComponentFlag_ModelComponent; }
		static constexpr int GetIndex() { return Math::floorlog2(GetFlagType()); }

		std::string m_Path;
	};
}