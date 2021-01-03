#pragma once

#include "Core/Math/MathFunc.h"

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

namespace Dodo {

	namespace Math {

		//////////// Vec2 ////////////

		// Sum of Vec2
		static inline float Sum(const TVec2<float>& vec)
		{
			return vec.x + vec.y;
		}

		// Dot product of two Vec2
		static inline float Dot(const TVec2<float>& first, const TVec2<float>& second)
		{
			return first.x * first.x + first.y * first.y;
		}

		// Get distance between two Vec2
		static inline float Distance(const TVec2<float>& first, const TVec2<float>& second)
		{
			return sqrt(
				(first.x - second.x) * (first.x - second.x) +
				(first.y - second.y) * (first.y - second.y));
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

		// Turn vec2 using degrees into vec2 using radians
		static inline const TVec2<float> ToRadians(const TVec2<float>& degrees)
		{
			return degrees * MATH_PI / 180.0f;
		}

		// Turn vec2 using radians into vec2 using degrees
		static inline const TVec2<float> ToDegrees(const TVec2<float>& radians)
		{
			return radians * 180.0f / MATH_PI;
		}

		//////////// Vec3 ////////////

		// Sum of Vec3
		static inline float Sum(const TVec3<float>& vec)
		{
			return vec.x + vec.y + vec.z;
		}

		// Dot product of two Vec3
		static inline float Dot(const TVec3<float>& first, const TVec3<float>& second)
		{
			return first.x * second.x + first.y * second.y + first.z * second.z;
		}

		// Get distance between two Vec3
		static inline float Distance(const TVec3<float>& first, const TVec3<float>& second)
		{
			return sqrt(
				(first.x - second.x) * (first.x - second.x) +
				(first.y - second.y) * (first.y - second.y) +
				(first.z - second.z) * (first.z - second.z));
		}

		// Use when values are above 1.0f
		static inline const TVec3<float> Normalize(const TVec3<float>& vec)
		{
			return vec.Normalize();
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

		// Turn vec3 using degrees into vec3 using radians
		static inline const TVec3<float> ToRadians(const TVec3<float>& degrees)
		{
			return degrees * MATH_PI / 180.0f;
		}

		// Turn vec3 using radians into vec3 using degrees
		static inline const TVec3<float> ToDegrees(const TVec3<float>& radians)
		{
			return radians * 180.0f / MATH_PI;
		}

		//////////// Vec4 ////////////

		// Sum of Vec4
		static inline float Sum(const TVec4<float>& vec)
		{
			return vec.x + vec.y + vec.z + vec.w;
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

		// Turn vec4 using degrees into vec4 using radians
		static inline const TVec4<float> ToRadians(const TVec4<float>& degrees)
		{
			return degrees * (MATH_PI / 180.0f);
		}

		// Turn vec4 using radians into vec4 using degrees
		static inline const TVec4<float> ToDegrees(const TVec4<float>& radians)
		{
			return radians * (180.0f / MATH_PI);
		}
	}
}