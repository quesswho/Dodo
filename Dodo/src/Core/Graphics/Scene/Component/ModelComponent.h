#pragma once

#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"

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

		void Draw(const Math::Mat4& camera) const;

		std::string m_Path;
	};
}