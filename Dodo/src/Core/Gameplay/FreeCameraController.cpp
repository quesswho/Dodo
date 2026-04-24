#include "FreeCameraController.h"

#include "Core/Application/Application.h"

namespace Dodo {
    FreeCameraController::FreeCameraController(const Math::Vec3& pos, float yaw, float pitch, float aspectRatio,
                                               float sensitivity, float speed, float friction)
        : m_Camera(pos, yaw, pitch, aspectRatio), m_Sensitivity(sensitivity), m_Speed(speed), m_Friction(friction),
          m_Velocity(0.0f)
    {
        m_LastMousePos = Math::TVec2<double>(Application::s_Application->m_RenderAPI->m_ViewportPosX +
                                                 Application::s_Application->m_RenderAPI->m_ViewportWidth / 2.0,
                                             Application::s_Application->m_RenderAPI->m_ViewportPosY +
                                                 Application::s_Application->m_RenderAPI->m_ViewportHeight / 2.0);
        Application::s_Application->m_Window->SetCursorPosition(m_LastMousePos);
        m_MouseRect =
            Math::TVec4<double>(Application::s_Application->m_RenderAPI->m_ViewportPosX +
                                    Application::s_Application->m_RenderAPI->m_ViewportWidth / 4.0,
                                Application::s_Application->m_RenderAPI->m_ViewportPosX +
                                    (int)(Application::s_Application->m_RenderAPI->m_ViewportWidth * (3.0 / 4.0)),
                                Application::s_Application->m_RenderAPI->m_ViewportPosY +
                                    Application::s_Application->m_RenderAPI->m_ViewportHeight / 4.0,
                                Application::s_Application->m_RenderAPI->m_ViewportPosY +
                                    Application::s_Application->m_RenderAPI->m_ViewportHeight * (int)(3.0 / 4.0));
    }

    void FreeCameraController::ResetMouse()
    {
        m_LastMousePos = Math::TVec2<double>(Application::s_Application->m_RenderAPI->m_ViewportPosX +
                                                 Application::s_Application->m_RenderAPI->m_ViewportWidth / 2.0,
                                             Application::s_Application->m_RenderAPI->m_ViewportPosY +
                                                 Application::s_Application->m_RenderAPI->m_ViewportHeight / 2.0);
        Application::s_Application->m_Window->SetCursorPosition(m_LastMousePos);
    }

    void FreeCameraController::Resize(uint width, uint height)
    {
        m_Camera.Resize(width, height);
    }

    void FreeCameraController::Update(const Input& input, float elapsed)
    {
        Math::Vec3 forward = m_Camera.GetForward();
        Math::Vec3 right = m_Camera.GetRight();
        Math::Vec3 up = m_Camera.GetUp();

        Math::Vec3 inputDir = Math::Vec3(0.0f);
        if (input.IsKeyPressed(DODO_KEY_W)) inputDir += forward;
        if (input.IsKeyPressed(DODO_KEY_S)) inputDir -= forward;
        if (input.IsKeyPressed(DODO_KEY_D)) inputDir += right;
        if (input.IsKeyPressed(DODO_KEY_A)) inputDir -= right;

        // Normalize movement vector to prevent faster diagonal movement and keep up and down movement at the same speed
        // as horizontal movement
        inputDir.NormalizeVector();

        if (input.IsKeyPressed(DODO_KEY_SPACE)) inputDir += up;
        if (input.IsKeyPressed(DODO_KEY_LEFT_CONTROL)) inputDir -= up;

        m_Velocity = Math::ExpDecay(m_Velocity, inputDir * m_Speed, m_Friction, elapsed);
        m_Camera.Move(m_Velocity * elapsed);
    }

    void FreeCameraController::UpdateRotation(const Input& input)
    {
        if (!Application::s_Application->m_Window->m_Focused) return;

        Math::TVec2<double> mousePos = input.GetMousePosition();
        double movementX = mousePos.x - m_LastMousePos.x;
        double movementY = m_LastMousePos.y - mousePos.y;

        // Keep mouse inside rectangle
        if (m_LastMousePos.x < m_MouseRect.x || m_LastMousePos.x > m_MouseRect.y || m_LastMousePos.y < m_MouseRect.z ||
            m_LastMousePos.y > m_MouseRect.w) {
            m_LastMousePos = Math::TVec2<double>(Application::s_Application->m_RenderAPI->m_ViewportPosX +
                                                     Application::s_Application->m_RenderAPI->m_ViewportWidth / 2,
                                                 Application::s_Application->m_RenderAPI->m_ViewportPosY +
                                                     Application::s_Application->m_RenderAPI->m_ViewportHeight / 2);
            Application::s_Application->m_Window->SetCursorPosition(m_LastMousePos);
        } else {
            m_LastMousePos = mousePos;
        }

        movementX *= m_Sensitivity;
        movementY *= m_Sensitivity;

        m_Camera.Rotate((float)movementX, (float)movementY);
    }
} // namespace Dodo