#pragma once

#include "Mat4.h"

namespace Dodo {

	namespace Math {

		struct Transformation {

			Math::Vec3 m_Pos;
			Math::Vec3 m_Scale;
			Math::Vec3 m_Axis;
			float m_Radians;

			Math::Mat4 m_Model;

			Transformation()
				: m_Pos(Vec3(0.0f)), m_Scale(Vec3(1.0f)), m_Axis(Vec3(0.0f, 1.0f, 0.0f)), m_Radians(0.0f)
			{
				Calculate();
			}

			Transformation(const Vec3& pos, const Vec3& scale, const Vec3& axis, float radians)
				: m_Pos(pos), m_Scale(scale), m_Axis(axis), m_Radians(radians)
			{
				Calculate();
			}

			Transformation(const Vec3& pos, const Vec3& scale)
				: m_Pos(pos), m_Scale(scale), m_Axis(Vec3(0.0f,1.0f, 0.0f)), m_Radians(0.0f)
			{
				Calculate();
			}

			inline void Move(const Vec3& pos)
			{
				m_Pos = pos;
				Calculate();
			}

			inline void Scale(const Vec3& scale)
			{
				m_Scale = scale;
				Calculate();
			}

			inline void Rotate(float radians, const Vec3& axis)
			{
				m_Radians = radians;
				m_Axis = axis;
				Calculate();
			}

			inline void Transformate(const Math::Vec3& pos, const Math::Vec3& scale, const Math::Vec3& axis, float radians)
			{
				m_Pos = pos;
				m_Scale = scale;
				m_Radians = radians;
				m_Axis = axis;
				Calculate();
			}

			inline void Calculate()
			{
				m_Model = Mat4(1.0f);
				m_Model = Mat4::Translate(m_Pos) * Mat4::Rotate(m_Radians, m_Axis) * Mat4::Scale(m_Scale);
			}
		};
	}
}