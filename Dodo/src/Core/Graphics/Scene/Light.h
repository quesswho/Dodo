#pragma once

#include "Core/Math/Vector/Vec3.h"

namespace Dodo {

	struct LightSystem {
		struct DirectionalLight {
			DirectionalLight()
				: m_Direction(Math::Vec3(0.0f, -1.0f, 0.0f))
			{}

			Math::Vec3 m_Direction;
			Math::Mat4 m_LightCamera;
		};

		DirectionalLight m_Directional;

		// Eventually point lights here
	};
}
