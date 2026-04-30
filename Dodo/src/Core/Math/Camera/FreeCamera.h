#pragma once

#include "Core/Math/Matrix/Mat4.h"

namespace Dodo::Math {

    class FreeCamera {
      public:
        FreeCamera(const Vec3& pos, float yaw, float pitch, float aspectRatio);

        float GetNearPlane()   const { return m_NearPlane; }
        float GetFarPlane()    const { return m_FarPlane; }
        float GetFov()         const { return m_Fov; }
        float GetAspectRatio() const { return m_AspectRatio; }

        void SetPosition(const Vec3& pos);
        void Move(const Vec3& delta);
        void Rotate(float yaw, float pitch);
        void Resize(uint width, uint height);

        Mat4 GetViewMatrix() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_ViewMatrix;
        }
        Mat4 GetProjectionMatrix() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_ProjectionMatrix;
        }
        Mat4 GetCameraMatrix() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_CameraMatrix;
        }
        Vec3 GetPosition() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_CameraPos;
        }
        Vec3 GetForward() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_Forward;
        }
        Vec3 GetRight() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_Right;
        }
        Vec3 GetUp() const
        {
            if (m_Dirty) CalculateCameraMatrix();
            return m_Up;
        }

      private:
        void CalculateCameraMatrix() const;
        // This is mutable because of the lazy evaluation logic and we want to be able to call the getters from const
        // contexts
        mutable bool m_Dirty = true;
        mutable Mat4 m_ViewMatrix;
        mutable Mat4 m_CameraMatrix;
        mutable Vec3 m_ViewDir;
        mutable Vec3 m_Forward;
        mutable Vec3 m_Right;
        mutable Vec3 m_Up;

        // These are never modified in const contexts
        Mat4 m_ProjectionMatrix;
        Vec3 m_CameraPos;
        Vec3 m_WorldUp;
        float m_Yaw, m_Pitch;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.0f;
        float m_Fov = 45.0f;
        float m_AspectRatio = 1.0f;
    };
} // namespace Dodo::Math