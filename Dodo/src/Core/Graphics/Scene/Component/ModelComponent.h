#pragma once

#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"

namespace Dodo {

	class ModelComponent {
	private:
		Model* m_Model;

		Math::Transformation m_Transformation;
	public:
		ModelComponent(const char* path)
			: ModelComponent(path, Math::Transformation())
		{}

		ModelComponent(const char* path, const Math::Transformation& transformation);

		~ModelComponent();

		void Move(const Math::Vec3& pos);
		void Scale(const Math::Vec3& scale);
		void Rotate(float radians, const Math::Vec3& axis);

		void Transformation(const Math::Vec3& pos, const Math::Vec3& scale, const Math::Vec3& axis, float radians);

		void Draw() const;

		std::string m_Path;
	};
}