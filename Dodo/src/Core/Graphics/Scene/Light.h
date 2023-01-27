#pragma once

#include "Core/Math/Vector/Vec3.h"

namespace Dodo {

	struct LightSystem {
		struct DirectionalLight {
			Math::Vec3 m_Direction;
		};

		DirectionalLight m_Directional;

		// Eventually point lights here
	};
}
