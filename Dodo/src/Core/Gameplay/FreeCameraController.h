#pragma once

#include "Core/Math/Camera/FreeCamera.h"
#include "Core/Application/Input/Input.h"
namespace Dodo {

    class FreeCameraController {
      public:
        FreeCameraController(const Math::Vec3& pos, float yaw, float pitch, 
                             float aspectRatio, float sensitivity, float speed);
        void Update(Input& input, float elapsed);
        void UpdateRotation();
        void ResetMouse();
        void Resize(uint width, uint height) { m_Camera.Resize(width, height); }
        
        const Math::FreeCamera& GetCamera() const { return m_Camera; }
      private:
        Math::FreeCamera m_Camera;
        float m_Sensitivity, m_Speed;
        Math::TVec2<double> m_LastMousePos;
        Math::TVec4<double> m_MouseRect;
    };
} // namespace Dodo