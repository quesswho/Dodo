#include "pch.h"
#include "FreeCamera.h"

#include "Core/Application/Application.h"

namespace Dodo {
	namespace Math {
		FreeCamera::FreeCamera(const Vec3& pos, const Vec3& viewDir, float aspectRatio, float sensitivity, float speed)
			: m_CameraPos(pos), m_ViewDir(viewDir), m_Yaw(-90.0f), m_Pitch(0.0f), m_Sensitivity(sensitivity), m_Speed(speed), m_Forward(0.0f, 0.0f, 0.0f), m_Right(1.0f, 0.0f, 0.0f), m_WorldUp(Vec3(0.0f, 1.0f, 0.0f))
		{
			m_ProjectionMatrix = Mat4::Perspective(45.0f, aspectRatio, 1.0f, 1000.0f);
			m_LastMousePos = TVec2<long>(Application::s_Application->m_RenderAPI->m_ViewportPosX + Application::s_Application->m_RenderAPI->m_ViewportWidth / 2, Application::s_Application->m_RenderAPI->m_ViewportPosY + Application::s_Application->m_RenderAPI->m_ViewportHeight / 2);
			Application::s_Application->m_Window->SetCursorPosition(TVec2<long>(m_LastMousePos.x, m_LastMousePos.y));
			CalculateProjectionViewMatrix();
			m_MouseRect = TVec4<int>(Application::s_Application->m_RenderAPI->m_ViewportPosX + Application::s_Application->m_RenderAPI->m_ViewportWidth / 4, Application::s_Application->m_RenderAPI->m_ViewportPosX + (int) (Application::s_Application->m_RenderAPI->m_ViewportWidth * (3.0 / 4.0)), Application::s_Application->m_RenderAPI->m_ViewportPosY + Application::s_Application->m_RenderAPI->m_ViewportHeight / 4, Application::s_Application->m_RenderAPI->m_ViewportPosY + Application::s_Application->m_RenderAPI->m_ViewportHeight * (int)(3.0 / 4.0));
		}

		void FreeCamera::ResetMouse()
		{
			m_LastMousePos = TVec2<long>(Application::s_Application->m_RenderAPI->m_ViewportPosX + Application::s_Application->m_RenderAPI->m_ViewportWidth / 2, Application::s_Application->m_RenderAPI->m_ViewportPosY + Application::s_Application->m_RenderAPI->m_ViewportHeight / 2);
			Application::s_Application->m_Window->SetCursorPosition(TVec2<long>(m_LastMousePos.x, m_LastMousePos.y));
		}

		void FreeCamera::Resize(uint width, uint height)
		{
			m_ProjectionMatrix = Mat4::Perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
			m_MouseRect = TVec4<int>(Application::s_Application->m_RenderAPI->m_ViewportPosX + width / 4, Application::s_Application->m_RenderAPI->m_ViewportPosX + (int)(width * (3.0 / 4.0)), Application::s_Application->m_RenderAPI->m_ViewportPosY + height / 4, Application::s_Application->m_RenderAPI->m_ViewportPosY + (int)((double)height * (3.0 / 4.0)));
			CalculateProjectionViewMatrix();
		}

		void FreeCamera::Update(float elapsed)
		{
			m_MoveDirection = Vec3();
			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_W))
				m_MoveDirection.x += 1.0f;
			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_S))
				m_MoveDirection.x -= 1.0f;
			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_D))
				m_MoveDirection.y += 1.0f;
			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_A))
				m_MoveDirection.y -= 1.0f;
			
			m_MoveDirection.NormalizeVector();

			m_CameraPos += m_MoveDirection.x * m_Forward * m_Speed * elapsed;
			m_CameraPos += m_MoveDirection.y * m_Right * m_Speed * elapsed;

			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_SPACE))
				m_CameraPos += m_Up * m_Speed * elapsed;
			if (Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_LEFT_CONTROL))
				m_CameraPos -= m_Up * m_Speed * elapsed;

			CalculateProjectionViewMatrix();
		}

		void FreeCamera::UpdateRotation()
		{
			if (Application::s_Application->m_Window->m_Focused)
			{

				float movementX = (float)Application::s_Application->m_Window->m_MousePos.x - m_LastMousePos.x;
				float movementY = (float)m_LastMousePos.y - Application::s_Application->m_Window->m_MousePos.y;

				if (m_LastMousePos.x < m_MouseRect.x || m_LastMousePos.x > m_MouseRect.y || m_LastMousePos.y < m_MouseRect.z || m_LastMousePos.y > m_MouseRect.w)
				{
					m_LastMousePos = TVec2<long>(Application::s_Application->m_RenderAPI->m_ViewportPosX + Application::s_Application->m_RenderAPI->m_ViewportWidth / 2, Application::s_Application->m_RenderAPI->m_ViewportPosY + Application::s_Application->m_RenderAPI->m_ViewportHeight / 2);
					Application::s_Application->m_Window->SetCursorPosition(TVec2<long>(m_LastMousePos.x, m_LastMousePos.y));
				}
				else
					m_LastMousePos = Application::s_Application->m_Window->m_MousePos;

				movementX *= m_Sensitivity;
				movementY *= m_Sensitivity;

				m_Yaw = std::fmod((m_Yaw + movementX), 360.0f); // Prevent yaw from reaching high numbers
				m_Pitch += movementY;

				if (m_Pitch > 89.9f)
					m_Pitch = 89.9f;
				if (m_Pitch < -89.0f)
					m_Pitch = -89.0f;
			}
		}

		void FreeCamera::CalculateProjectionViewMatrix()
		{
			m_Forward = Normalize(Vec3(cos(ToRadians(m_Yaw)), 0.0f, sin(ToRadians(m_Yaw))));
			m_ViewDir = Normalize(Vec3(cos(ToRadians(m_Yaw)) * cos(ToRadians(m_Pitch)), sin(ToRadians(m_Pitch)), sin(ToRadians(m_Yaw)) * cos(ToRadians(m_Pitch))));
			m_Right = Normalize(Cross(m_Forward, m_WorldUp));
			m_Up = Normalize(Cross(m_Right, m_Forward));
			
			m_ViewMatrix = Mat4::LookDir(m_CameraPos, m_ViewDir, m_Up);
			m_CameraMatrix = Mat4::Multiply(m_ProjectionMatrix, m_ViewMatrix);
		}
	}
}