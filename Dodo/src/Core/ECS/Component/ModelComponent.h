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
            for(auto mesh : m_Model->GetMeshes())
            {
                Ref<Material> mat = mesh->GetMaterial();
                mat->Bind();
                mat->SetUniform("u_LightCamera", lights.m_Directional.m_LightCamera);
                mat->SetUniform("u_LightDir", lights.m_Directional.m_Direction);
                mat->SetUniform("u_Model", m_Transformation.m_Model);
                mat->SetUniform("u_Camera", camera->GetCameraMatrix());
                mat->SetUniform("u_CameraPos", camera->GetCameraPos());
                mesh->DrawGeometry();
            }
		}

		void Draw(Ref<Material> material) const
		{
			material->Bind();
			material->SetUniform("u_Model", m_Transformation.m_Model);
			m_Model->Draw(material);
		}

		static inline bool IsDrawable() { return true; }
		static inline std::string GetName() { return std::string("ModelComponent"); }
		static constexpr ComponentFlag GetFlagType() { return ComponentFlag::ComponentFlag_ModelComponent; }
		static constexpr int GetIndex() { return Math::floorlog2(GetFlagType()); }

		std::string m_Path;
	};
}