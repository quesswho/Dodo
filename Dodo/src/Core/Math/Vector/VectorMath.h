#pragma once

#include "Core/Math/MathFunc.h"

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

namespace Dodo {

	namespace Math {

		//////////// Vec2 ////////////


		// Dot product of two Vec2
		static inline float Dot(const TVec2<float>& first, const TVec2<float>& second)
		{
			return first.x * first.x + first.y * first.y;
		}

		// Normalize a Vec2
		static inline const TVec2<float> Normalize(const TVec2<float>& vec)
		{
			float mag = vec.Magnitude();
			if (mag > 0)
				return vec * (1.0f / mag);
			return vec;
		}

		// Use when values are above 1.0f
		static inline const TVec2<float> fast_Normalize(const TVec2<float>& vec)
		{
			return vec * fast_isqrt(Dot(vec, vec));
		}

		// Get distance between two Vec2
		static inline float Distance(const TVec2<float>& first, const TVec2<float>& second)
		{
			return sqrt(
				(first.x - second.x) * (first.x - second.x) +
				(first.y - second.y) * (first.y - second.y));
		}

		//////////// Vec3 ////////////



		// Dot product of two Vec3
		static inline float Dot(const TVec3<float>& first, const TVec3<float>& second)
		{
			return first.x * second.x + first.y * second.y + first.z * second.z;
		}

		// Normalize a Vec3
		static inline const TVec3<float> Normalize(const TVec3<float>& vec)
		{
			float mag = vec.Magnitude();
			if (mag > 0)
				return vec * (1.0f / mag);
			return vec;
		}

		// Use when values are above 1.0f
		static inline const TVec3<float> fast_Normalize(const TVec3<float>& vec)
		{
			return vec * fast_isqrt(Dot(vec, vec));
		}

		// Normalize and negate a Vec3
		static inline const TVec3<float> NegativeNormalize(const TVec3<float>& vec)
		{
			float mag = vec.Magnitude();
			if (mag > 0)
				return vec * -(1.0f / mag);
			return vec;
		}

		// Cross product of two Vec3
		static inline TVec3<float> Cross(const TVec3<float>& first, const TVec3<float>& second)
		{
			return TVec3<float>(first.y * second.z - first.z * second.y, first.z * second.x - first.x * second.z, first.x * second.y - first.y * second.x);
		}

		// Get distance between two Vec3
		static inline float Distance(const TVec3<float>& first, const TVec3<float>& second)
		{
			return sqrt(
				(first.x - second.x) * (first.x - second.x) +
				(first.y - second.y) * (first.y - second.y) +
				(first.z - second.z) * (first.z - second.z));
		}

		//////////// Vec4 ////////////

		// Dot product of two Vec4
		static inline float Dot(const TVec4<float>& first, const TVec4<float>& second)
		{
			return first.x * second.x + first.y * second.y + first.z * second.z + first.w * second.w;
		}

		// Normalize a Vec4
		static inline const TVec4<float> Normalize(const TVec4<float>& vec)
		{
			float mag = vec.Magnitude();
			if (mag > 0)
				return vec * (1 / mag);
			return vec;
		}

		// Use when values are above 1.0f
		static inline const TVec4<float> fast_Normalize(const TVec4<float>& vec)
		{
			return vec * fast_isqrt(Dot(vec, vec));
		}


		// Get distance between two Vec4
		static inline float Distance(const TVec4<float>& first, const TVec4<float>& second)
		{
			return sqrt(
				(first.x - second.x) * (first.x - second.x) +
				(first.y - second.y) * (first.y - second.y) +
				(first.z - second.z) * (first.z - second.z) +
				(first.w - second.w) * (first.w - second.w));
		}
	}
}