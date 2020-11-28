#pragma once

#include "Components.h"
#include "Core/Graphics/Scene/Rectangle.h"
#include "Core/Math/Matrix/Transformation.h"

namespace Dodo {

	class Rectangle2DComponent {
	private:
		Rectangle* m_Rectangle;
	public:
		Math::Transformation m_Transformation;

		Rectangle2DComponent(const char* path)
			: Rectangle2DComponent(path, Math::Transformation())
		{}

		Rectangle2DComponent(const char* path, const Math::Transformation& transformation);

		~Rectangle2DComponent();

		template<typename T>
		void Draw(const T* camera) const
		{
			m_Rectangle->Bind();
			m_Rectangle->SetUniform("u_Model", m_Transformation.m_Model);
			m_Rectangle->SetUniform("u_Camera", camera->GetCameraMatrix());
			m_Rectangle->SetUniform("u_CameraPos", camera->GetCameraPos());
			m_Rectangle->Draw();
		}

		static inline const std::string& GetName() { return std::string("ModelComponent"); }
		static inline const ComponentFlag GetFlagType() { return ComponentFlag::ComponentFlag_Rectangle2DComponent; }
		std::string m_Path;
	};
}