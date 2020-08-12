#pragma once

#pragma once

#include "Core/Math/Matrix/Mat3.h"
#include "Core/Math/Vector/Vec4.h"
#include "Core/Math/MathFunc.h"

namespace Dodo {
	namespace Platform {

		// Column Major Right hand 4x4 matrix
		template<typename T = float>
		struct CRHMat4x4 {

			union {
				T m_Elements[16];
				Math::TVec4<T> m_Columns[4];
			};

			CRHMat4x4() 
				: m_Elements { 0 }
			{}
			CRHMat4x4(float diagonal) { Identity(diagonal); }

			CRHMat4x4(const Math::TVec4<T>& first, const Math::TVec4<T>& second, const Math::TVec4<T>& third, const Math::TVec4<T>& forth)
			{
				m_Elements[GetIndex(0, 0)] = first.x;
				m_Elements[GetIndex(0, 1)] = first.y;
				m_Elements[GetIndex(0, 2)] = first.z;
				m_Elements[GetIndex(0, 3)] = first.w;

				m_Elements[GetIndex(1, 0)] = second.x;
				m_Elements[GetIndex(1, 1)] = second.y;
				m_Elements[GetIndex(1, 2)] = second.z;
				m_Elements[GetIndex(1, 3)] = second.w;

				m_Elements[GetIndex(2, 0)] = third.x;
				m_Elements[GetIndex(2, 1)] = third.y;
				m_Elements[GetIndex(2, 2)] = third.z;
				m_Elements[GetIndex(2, 3)] = third.w;

				m_Elements[GetIndex(3, 0)] = forth.x;
				m_Elements[GetIndex(3, 1)] = forth.y;
				m_Elements[GetIndex(3, 2)] = forth.z;
				m_Elements[GetIndex(3, 3)] = forth.w;
			}

			CRHMat4x4(T first, T second, T third, T forth, T fifth, T sixth, T seventh, T eighth, T ninth, T tenth, T eleventh, T twelveth, T thirteenth, T fourteenth, T fifteenth, T sixteenth)
			{
				m_Elements[GetIndex(0, 0)] = first;
				m_Elements[GetIndex(0, 1)] = second;
				m_Elements[GetIndex(0, 2)] = third;
				m_Elements[GetIndex(0, 3)] = forth;

				m_Elements[GetIndex(1, 0)] = fifth;
				m_Elements[GetIndex(1, 1)] = sixth;
				m_Elements[GetIndex(1, 2)] = seventh;
				m_Elements[GetIndex(1, 3)] = eighth;

				m_Elements[GetIndex(2, 0)] = ninth;
				m_Elements[GetIndex(2, 1)] = tenth;
				m_Elements[GetIndex(2, 2)] = eleventh;
				m_Elements[GetIndex(2, 3)] = twelveth;

				m_Elements[GetIndex(3, 0)] = thirteenth;
				m_Elements[GetIndex(3, 1)] = fourteenth;
				m_Elements[GetIndex(3, 2)] = fifteenth;
				m_Elements[GetIndex(3, 3)] = sixteenth;
			}

			CRHMat4x4(const Math::Mat3x3<T>& mat)
			{
				IdentityRH(1.0f);
				m_Elements[GetIndex(0, 0)] = mat.m_Elements[0];
				m_Elements[GetIndex(0, 1)] = mat.m_Elements[1];
				m_Elements[GetIndex(0, 2)] = mat.m_Elements[2];

				m_Elements[GetIndex(1, 0)] = mat.m_Elements[3];
				m_Elements[GetIndex(1, 1)] = mat.m_Elements[4];
				m_Elements[GetIndex(1, 2)] = mat.m_Elements[5];

				m_Elements[GetIndex(2, 0)] = mat.m_Elements[6];
				m_Elements[GetIndex(2, 1)] = mat.m_Elements[7];
				m_Elements[GetIndex(2, 2)] = mat.m_Elements[8];
			}

			inline void Identity(float diagonal = 1.0f)
			{
				memset(m_Elements, 0, 16 * 4);

				m_Elements[GetIndex(0, 0)] = (T)diagonal;
				m_Elements[GetIndex(1, 1)] = (T)diagonal;
				m_Elements[GetIndex(2, 2)] = (T)diagonal;
				m_Elements[GetIndex(3, 3)] = (T)diagonal;
			}

			inline T operator[](int i) const { this->m_Elements[i]; }

			// Addition

			const CRHMat4x4& operator+=(const float scalar)
			{
				for (int i = 0; i < 16; i++)
				{
					this->m_Elements[i] += scalar
				}
				return *this;
			}

			friend CRHMat4x4 operator+(CRHMat4x4 left, const float right) { return left += right; }

			const CRHMat4x4& operator+=(const CRHMat4x4& other)
			{
				for (int i = 0; i < 16; i++)
				{
					this->m_Elements[i] += other.m_Elements[i];
				}
				return *this;
			}

			friend CRHMat4x4 operator+(CRHMat4x4 left, const CRHMat4x4& right) { return left += right; }

			// Subtraction

			const CRHMat4x4& operator-=(const float scalar)
			{
				for (int i = 0; i < 16; i++)
				{
					this->m_Elements[i] -= scalar
				}
				return *this;
			}

			friend CRHMat4x4 operator-(CRHMat4x4 left, const float right) { return left -= right; }

			const CRHMat4x4& operator-=(const CRHMat4x4& other)
			{
				for (int i = 0; i < 16; i++)
				{
					this->m_Elements[i] -= other.m_Elements[i];
				}
				return *this;
			}

			friend CRHMat4x4 operator-(CRHMat4x4 left, const CRHMat4x4& right) { return left -= right; }

			// Multiplication

			const CRHMat4x4& operator*=(const float scalar)
			{
				for (int i = 0; i < 16; i++)
					m_Elements[i] *= scalar;

				return *this;
			}

			friend CRHMat4x4 operator*(CRHMat4x4 left, const float right) { return left *= right; }


			inline const CRHMat4x4& operator*=(const CRHMat4x4& other)
			{

				const Math::TVec4 row0 = Math::TVec4(m_Elements[GetIndex(0, 0)], m_Elements[GetIndex(1, 0)], m_Elements[GetIndex(2, 0)], m_Elements[GetIndex(3, 0)]);
				const Math::TVec4 row1 = Math::TVec4(m_Elements[GetIndex(0, 1)], m_Elements[GetIndex(1, 1)], m_Elements[GetIndex(2, 1)], m_Elements[GetIndex(3, 1)]);
				const Math::TVec4 row2 = Math::TVec4(m_Elements[GetIndex(0, 2)], m_Elements[GetIndex(1, 2)], m_Elements[GetIndex(2, 2)], m_Elements[GetIndex(3, 2)]);
				const Math::TVec4 row3 = Math::TVec4(m_Elements[GetIndex(0, 3)], m_Elements[GetIndex(1, 3)], m_Elements[GetIndex(2, 3)], m_Elements[GetIndex(3, 3)]);

				m_Elements[0] = other.m_Columns[0].Dot(row0);
				m_Elements[1] = other.m_Columns[0].Dot(row1);
				m_Elements[2] = other.m_Columns[0].Dot(row2);
				m_Elements[3] = other.m_Columns[0].Dot(row3);

				m_Elements[4] = other.m_Columns[1].Dot(row0);
				m_Elements[5] = other.m_Columns[1].Dot(row1);
				m_Elements[6] = other.m_Columns[1].Dot(row2);
				m_Elements[7] = other.m_Columns[1].Dot(row3);

				m_Elements[8] = other.m_Columns[2].Dot(row0);
				m_Elements[9] = other.m_Columns[2].Dot(row1);
				m_Elements[10] = other.m_Columns[2].Dot(row2);
				m_Elements[11] = other.m_Columns[2].Dot(row3);

				m_Elements[12] = other.m_Columns[3].Dot(row0);
				m_Elements[13] = other.m_Columns[3].Dot(row1);
				m_Elements[14] = other.m_Columns[3].Dot(row2);
				m_Elements[15] = other.m_Columns[3].Dot(row3);

				return *this;
			}

			friend CRHMat4x4 operator*(CRHMat4x4 left, const CRHMat4x4& right) { return left *= right; }


			// Assignment

			const CRHMat4x4& operator=(const CRHMat4x4& other)
			{
				m_Elements[GetIndex(0, 0)] = other.m_Elements[GetIndex(0, 0)];
				m_Elements[GetIndex(0, 1)] = other.m_Elements[GetIndex(0, 1)];
				m_Elements[GetIndex(0, 2)] = other.m_Elements[GetIndex(0, 2)];
				m_Elements[GetIndex(0, 3)] = other.m_Elements[GetIndex(0, 3)];

				m_Elements[GetIndex(1, 0)] = other.m_Elements[GetIndex(1, 0)];
				m_Elements[GetIndex(1, 1)] = other.m_Elements[GetIndex(1, 1)];
				m_Elements[GetIndex(1, 2)] = other.m_Elements[GetIndex(1, 2)];
				m_Elements[GetIndex(1, 3)] = other.m_Elements[GetIndex(1, 3)];

				m_Elements[GetIndex(2, 0)] = other.m_Elements[GetIndex(2, 0)];
				m_Elements[GetIndex(2, 1)] = other.m_Elements[GetIndex(2, 1)];
				m_Elements[GetIndex(2, 2)] = other.m_Elements[GetIndex(2, 2)];
				m_Elements[GetIndex(2, 3)] = other.m_Elements[GetIndex(2, 3)];

				m_Elements[GetIndex(3, 0)] = other.m_Elements[GetIndex(3, 0)];
				m_Elements[GetIndex(3, 1)] = other.m_Elements[GetIndex(3, 1)];
				m_Elements[GetIndex(3, 2)] = other.m_Elements[GetIndex(3, 2)];
				m_Elements[GetIndex(3, 3)] = other.m_Elements[GetIndex(3, 3)];
				return *this;
			}

			// Test

			const bool operator==(const CRHMat4x4& other) const
			{
				return (
					m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
					m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
					m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&
					m_Elements[GetIndex(0, 3)] == other.m_Elements[GetIndex(0, 3)] &&

					m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
					m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
					m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&
					m_Elements[GetIndex(1, 3)] == other.m_Elements[GetIndex(1, 3)] &&

					m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
					m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
					m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)] &&
					m_Elements[GetIndex(2, 3)] == other.m_Elements[GetIndex(2, 3)] &&

					m_Elements[GetIndex(3, 0)] == other.m_Elements[GetIndex(3, 0)] &&
					m_Elements[GetIndex(3, 1)] == other.m_Elements[GetIndex(3, 1)] &&
					m_Elements[GetIndex(3, 2)] == other.m_Elements[GetIndex(3, 2)] &&
					m_Elements[GetIndex(3, 3)] == other.m_Elements[GetIndex(3, 3)]);
			}

			const bool operator!=(const CRHMat4x4& other) const
			{
				return !(
					m_Elements[GetIndex(0, 0)] == other.m_Elements[GetIndex(0, 0)] &&
					m_Elements[GetIndex(0, 1)] == other.m_Elements[GetIndex(0, 1)] &&
					m_Elements[GetIndex(0, 2)] == other.m_Elements[GetIndex(0, 2)] &&
					m_Elements[GetIndex(0, 3)] == other.m_Elements[GetIndex(0, 3)] &&

					m_Elements[GetIndex(1, 0)] == other.m_Elements[GetIndex(1, 0)] &&
					m_Elements[GetIndex(1, 1)] == other.m_Elements[GetIndex(1, 1)] &&
					m_Elements[GetIndex(1, 2)] == other.m_Elements[GetIndex(1, 2)] &&
					m_Elements[GetIndex(1, 3)] == other.m_Elements[GetIndex(1, 3)] &&

					m_Elements[GetIndex(2, 0)] == other.m_Elements[GetIndex(2, 0)] &&
					m_Elements[GetIndex(2, 1)] == other.m_Elements[GetIndex(2, 1)] &&
					m_Elements[GetIndex(2, 2)] == other.m_Elements[GetIndex(2, 2)] &&
					m_Elements[GetIndex(2, 3)] == other.m_Elements[GetIndex(2, 3)] &&

					m_Elements[GetIndex(3, 0)] == other.m_Elements[GetIndex(3, 0)] &&
					m_Elements[GetIndex(3, 1)] == other.m_Elements[GetIndex(3, 1)] &&
					m_Elements[GetIndex(3, 2)] == other.m_Elements[GetIndex(3, 2)] &&
					m_Elements[GetIndex(3, 3)] == other.m_Elements[GetIndex(3, 3)]);
			}

			///////
			// Useful Matrices
			///////

			static const CRHMat4x4 Transpose(const CRHMat4x4 mat) {
				return CRHMat4x4(
					mat.m_Elements[GetIndex(0, 0)], mat.m_Elements[GetIndex(1, 0)], mat.m_Elements[GetIndex(2, 0)], mat.m_Elements[GetIndex(3, 0)],
					mat.m_Elements[GetIndex(0, 1)], mat.m_Elements[GetIndex(1, 1)], mat.m_Elements[GetIndex(2, 1)], mat.m_Elements[GetIndex(3, 1)],
					mat.m_Elements[GetIndex(0, 2)], mat.m_Elements[GetIndex(1, 2)], mat.m_Elements[GetIndex(2, 2)], mat.m_Elements[GetIndex(3, 2)],
					mat.m_Elements[GetIndex(0, 3)], mat.m_Elements[GetIndex(1, 3)], mat.m_Elements[GetIndex(2, 3)], mat.m_Elements[GetIndex(3, 3)]);
			}

			static inline const CRHMat4x4<T> Translate(const Math::TVec3<T>& translation)
			{
				CRHMat4x4 result(1.0f);

				result.m_Elements[GetIndex(3, 0)] = translation.x;
				result.m_Elements[GetIndex(3, 1)] = translation.y;
				result.m_Elements[GetIndex(3, 2)] = translation.z;
				return result;
			}


			static inline const CRHMat4x4<T> Scale(const Math::TVec3<T>& scale)
			{
				CRHMat4x4 result(1.0f);

				result.m_Elements[GetIndex(0, 0)] = scale.x;
				result.m_Elements[GetIndex(1, 1)] = scale.y;
				result.m_Elements[GetIndex(2, 2)] = scale.z;
				return result;
			}


			// Recommended to normalize axis
			static inline CRHMat4x4<T> Rotate(const float radians, const Math::TVec3<T>& axis)
			{
				CRHMat4x4 result(1.0f);

				const float c = cos(radians);
				const float s = sin(radians);

				const Math::TVec3<T> temp((T(1) - c) * axis);

				result.m_Elements[GetIndex(0, 0)] = c + axis.x * temp.x;
				float aewqea = c + axis.x * temp.x;
				result.m_Elements[GetIndex(0, 1)] = s * axis.z + temp.x * axis.y;
				result.m_Elements[GetIndex(0, 2)] = -s * axis.y + temp.x * axis.z;

				result.m_Elements[GetIndex(1, 0)] = -s * axis.z + temp.y * axis.x;
				result.m_Elements[GetIndex(1, 1)] = c + axis.y * temp.y;
				result.m_Elements[GetIndex(1, 2)] = s * axis.x + temp.y * axis.z;

				result.m_Elements[GetIndex(2, 0)] = s * axis.y + temp.z * axis.x;
				result.m_Elements[GetIndex(2, 1)] = -s * axis.x + temp.z * axis.y;
				result.m_Elements[GetIndex(2, 2)] = c + axis.z * temp.z;

				return result;
			}

			// Infinite far plane
			static inline const CRHMat4x4<T> Orthographic(float left, float right, float bottom, float top)
			{
				return CRHMat4x4(
					2 / (right - left), 0, 0, 0,
					0, 2 / (top - bottom), 0, 0,
					0, 0, -1, 0,
					-(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1);
			}

			static inline const CRHMat4x4<T> Orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
			{

				/*	__												__
					| 2 / (r-l), 0,			0,			-(r+l)/(r-l) |
					| 0,		 2 / (t-b), 0,			-(t+b)/(t-b) |
					| 0,		 0,			-2 / (f-n), -(f+n)/(f-n) |
					| 0,		 0,			0,			1			 |
					¯¯												¯¯
				*/

				return CRHMat4x4(
					2 / (right - left), 0, 0, 0,
					0, 2 / (top - bottom), 0, 0,
					0, 0, -2 / (zFar - zNear), 0,
					-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1);
			}

			// Use degrees
			static inline const CRHMat4x4<T> Perspective(float fov, float aspectratio, float zNear, float zFar)
			{

				const float tanHalfFov = tan(Math::ToRadians(fov) / 2);

				/*	__														   __
					| s,		 0,			0,					        0		|
					| 0,		 s,			0,					        0		|
					| 0,		 0,			-f / (f - n),		-1		|
					| 0,		 0,			-f * n / (f - n), 0		|
					¯¯														   ¯¯
				*/


				return CRHMat4x4(
					1 / (aspectratio * tanHalfFov), 0, 0, 0,
					0, 1 / tanHalfFov, 0, 0,
					0, 0, -(zFar + zNear) / (zFar - zNear), -1,
					0, 0, -(2 * zFar * zNear) / (zFar - zNear), 0);
			}

			static inline const CRHMat4x4 LookAt(const Math::Vec3& eye, const Math::Vec3& to, const Math::Vec3& up)
			{
				CRHMat4x4 result(1.0f);

				const Vec3 f = Normalize(to - eye);
				const Vec3 r = Normalize(Cross(f, up));
				const Vec3 u = Cross(r, f);

				result.m_Elements[GetIndex(0, 0)] = r.x;
				result.m_Elements[GetIndex(1, 0)] = r.y;
				result.m_Elements[GetIndex(2, 0)] = r.z;
				result.m_Elements[GetIndex(0, 1)] = u.x;
				result.m_Elements[GetIndex(1, 1)] = u.y;
				result.m_Elements[GetIndex(2, 1)] = u.z;
				result.m_Elements[GetIndex(0, 2)] = -f.x;
				result.m_Elements[GetIndex(1, 2)] = -f.y;
				result.m_Elements[GetIndex(2, 2)] = -f.z;
				result.m_Elements[GetIndex(3, 0)] = -eye.Dot(r);
				result.m_Elements[GetIndex(3, 1)] = -eye.Dot(u);
				result.m_Elements[GetIndex(3, 2)] = eye.Dot(f);
				return result;
			}

			static inline const CRHMat4x4 LookDir(const Math::Vec3& pos, const Math::Vec3& dir, const Math::Vec3& up)
			{
				CRHMat4x4 result(1.0f);

				const Math::Vec3 r = Normalize(Cross(dir, up));
				const Math::Vec3 u = Cross(r, dir);

				result.m_Elements[GetIndex(0, 0)] = r.x;
				result.m_Elements[GetIndex(1, 0)] = r.y;
				result.m_Elements[GetIndex(2, 0)] = r.z;
				result.m_Elements[GetIndex(0, 1)] = u.x;
				result.m_Elements[GetIndex(1, 1)] = u.y;
				result.m_Elements[GetIndex(2, 1)] = u.z;
				result.m_Elements[GetIndex(0, 2)] = -dir.x;
				result.m_Elements[GetIndex(1, 2)] = -dir.y;
				result.m_Elements[GetIndex(2, 2)] = -dir.z;
				result.m_Elements[GetIndex(3, 0)] = -pos.Dot(r);
				result.m_Elements[GetIndex(3, 1)] = -pos.Dot(u);
				result.m_Elements[GetIndex(3, 2)] = pos.Dot(dir);
				return result;
				return result;
			}
			/////
			// Useful static functions
			/////
			
			static inline const CRHMat4x4<T> Multiply(const CRHMat4x4<T>& mat)
			{
				return mat;
			}

			template<class... O>
			static const CRHMat4x4<T> Multiply(const CRHMat4x4<T>& mat, O... others)
			{
				CRHMat4x4<T> result;
				result = mat * Multiply(others...);
				return result;
			}

			static inline const CRHMat4x4<T> RelinquishToMat3(const CRHMat4x4<T>& mat)
			{
				CRHMat4x4<T> result = mat;
				result.m_Elements[GetIndex(0, 3)] = 0.0f;
				result.m_Elements[GetIndex(1, 3)] = 0.0f;
				result.m_Elements[GetIndex(2, 3)] = 0.0f;
				result.m_Elements[GetIndex(3, 0)] = 0.0f;
				result.m_Elements[GetIndex(3, 1)] = 0.0f;
				result.m_Elements[GetIndex(3, 2)] = 0.0f;
				result.m_Elements[GetIndex(3, 3)] = 1.0f;
				return result;
			}

		private:
			static constexpr inline int GetIndex(int column, int row) { return (column * 4) + row; }
		};
	}
}
