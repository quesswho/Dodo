#pragma once

#include "Components.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"
#include "Core/Math/Camera/FreeCamera.h"

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

		void Draw(const Math::FreeCamera* camera) const;

		inline constexpr ComponentFlag GetFlagType() const { return ComponentFlag::ComponentFlag_ModelComponent; }

		std::string m_Path;
	};
}