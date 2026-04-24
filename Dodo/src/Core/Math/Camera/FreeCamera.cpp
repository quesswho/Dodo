#include "FreeCamera.h"

namespace Dodo::Math {
    FreeCamera::FreeCamera(const Vec3& pos, float yaw, float pitch, float aspectRatio)
        : m_CameraPos(pos), m_Yaw(yaw), m_Pitch(pitch), m_WorldUp(Vec3(0.0f, 1.0f, 0.0f))
    {
        m_ProjectionMatrix = Mat4::Perspective(45.0f, aspectRatio, 0.1f, 1000.0f);
        m_Dirty = true;
    }

    void FreeCamera::SetPosition(const Vec3& pos)
    {
        m_CameraPos = pos;
        m_Dirty = true;
    }

    void FreeCamera::Move(const Vec3& delta)
    {
        m_CameraPos += delta;
        m_Dirty = true;
    }

    void FreeCamera::Rotate(float yaw, float pitch)
    {
        m_Yaw = std::fmod((m_Yaw + yaw), 360.0f); // Prevent yaw from reaching high numbers
        m_Pitch += pitch;

        // Constrain the pitch to prevent gimbal lock
        if (m_Pitch > 89.0f) m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;

        m_Dirty = true;
    }

    void FreeCamera::Resize(uint width, uint height)
    {
        m_ProjectionMatrix = Mat4::Perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
        m_Dirty = true;
    }

    void FreeCamera::CalculateCameraMatrix() const
    {
        m_Forward = Normalize(Vec3(cos(ToRadians(m_Yaw)), 0.0f, sin(ToRadians(m_Yaw))));
        m_ViewDir = Normalize(Vec3(cos(ToRadians(m_Yaw)) * cos(ToRadians(m_Pitch)), sin(ToRadians(m_Pitch)),
                                   sin(ToRadians(m_Yaw)) * cos(ToRadians(m_Pitch))));
        m_Right = Normalize(Cross(m_Forward, m_WorldUp));
        m_Up = Normalize(Cross(m_Right, m_Forward));

        m_ViewMatrix = Mat4::LookDir(m_CameraPos, m_ViewDir, m_Up);
        m_CameraMatrix = Mat4::Multiply(m_ProjectionMatrix, m_ViewMatrix);
        m_Dirty = false;
    }
} // namespace Dodo::Math
