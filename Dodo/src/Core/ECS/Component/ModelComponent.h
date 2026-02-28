#pragma once

#include "Core/Graphics/Scene/Model.h"
#include "Core/Math/Matrix/Transformation.h"

namespace Dodo {

	struct ModelComponent {
		ModelID m_ModelID;
		Math::Transformation m_Transformation;

        ModelComponent() = default;
		explicit ModelComponent(ModelID modelID, const Math::Transformation& transformation = Math::Transformation())
			: m_ModelID(modelID), m_Transformation(transformation)
		{}
	};
}