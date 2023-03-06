#pragma once

#include <Core/Common.h>

#include <cmath>

#define MATH_PI 3.1415926535897932f

namespace Dodo {
	namespace Math {

		static inline constexpr float ToRadians(int degrees)
		{
			return degrees * MATH_PI / 180.0f;
		}

		static inline constexpr float ToRadians(float degrees)
		{
			return degrees * MATH_PI / 180.0f;
		}

		static inline constexpr float ToDegrees(float radians)
		{
			return radians * 180.0f / MATH_PI;
		}

		// Use only positive numbers
		static inline constexpr int fast_mod(const int input, const int ceil) {
			return input >= ceil ? input % ceil : input;
		}

		static constexpr unsigned int floorlog2(unsigned int x)
		{
			return x == 1 ? 0 : 1 + floorlog2(x >> 1);
		}
	}
}