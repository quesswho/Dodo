#pragma once

#include "Core/Application/Input/Input.h"
#include "Core/Math/Camera/FreeCamera.h"
namespace Dodo {

    class FreeCameraController {
      public:
        FreeCameraController(const Math::Vec3& pos, float yaw, float pitch, float aspectRatio, float sensitivity,
                             float speed, float friction = 20.0f);
        void Update(const Input& input, float elapsed);
        void UpdateRotation(const Input& input);
        void ResetMouse();
        void Resize(uint width, uint height);

        const Math::FreeCamera& GetCamera() const { return m_Camera; }

      private:
        Math::FreeCamera m_Camera;
        float m_Sensitivity, m_Speed, m_Friction;
        Math::TVec2<double> m_LastMousePos;
        Math::TVec4<double> m_MouseRect;
        Math::Vec3 m_Velocity;
    };
} // namespace Dodo