#pragma once

#include "Mat4.h"

namespace Dodo {

	namespace Math {

		struct Transformation {

			Math::Vec3 m_Position;
			Math::Vec3 m_Scale;
			Math::Vec3 m_Rotation;

			Math::Mat4 m_Model;

			Transformation()
				: m_Position(Vec3(0.0f)), m_Scale(Vec3(1.0f)), m_Rotation(Vec3(0.0f))
			{
				Calculate();
			}

			Transformation(const Vec3& pos, const Vec3& scale, const Vec3& rotate)
				: m_Position(pos), m_Scale(scale), m_Rotation(rotate)
			{
				Calculate();
			}

			Transformation(const Vec3& pos, const Vec3& scale)
				: m_Position(pos), m_Scale(scale), m_Rotation(Vec3(0.0f, 0.0f, 0.0f))
			{
				Calculate();
			}

			Transformation(float posX, float posY, float posZ, float scaleX, float scaleY, float scaleZ, float rotateX, float rotateY, float rotateZ)
				: m_Position(posX, posY, posZ), m_Scale(scaleX, scaleY, scaleZ), m_Rotation(rotateX, rotateY, rotateZ)
			{
				Calculate();
			}

			inline void Move(const Vec3& pos)
			{
				m_Position = pos;
				Calculate();
			}

			inline void Scale(const Vec3& scale)
			{
				m_Scale = scale;
				Calculate();
			}

			inline void Rotate(const Vec3& rotate)
			{
				m_Rotation = ToRadians(rotate);
				Calculate();
			}

			inline void Transformate(const Math::Vec3& pos, const Math::Vec3& scale, const Math::Vec3& rotate)
			{
				m_Position = pos;
				m_Scale = scale;
				m_Rotation = ToRadians(rotate);
				Calculate();
			}

			inline void Calculate()
			{
				m_Model = Mat4(1.0f);
				m_Model = Mat4::Translate(m_Position) * Mat4::Rotate(m_Rotation.z, Vec3(0.0f, 0.0f, 1.0f)) * Mat4::Rotate(m_Rotation.x, Vec3(1.0f, 0.0f, 0.0f)) * Mat4::Rotate(m_Rotation.y, Vec3(0.0f, 1.0f, 0.0f)) * Mat4::Scale(m_Scale);
			}
		};
	}
}