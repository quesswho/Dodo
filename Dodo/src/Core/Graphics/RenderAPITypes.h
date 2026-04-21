#pragma once

#include "Core/Math/Maths.h"
#include <string>
namespace Dodo {
    enum class RenderInitStatus {
        Success,
        Failed
    };

    struct RenderInitError {
        RenderInitError(RenderInitStatus status) : status(status) {}
        RenderInitError(RenderInitStatus status, const std::string& message) : status(status), message(message) {}
        RenderInitStatus status;
        std::string message;
    };

    struct FrameData {
        Math::Mat4 camera;
        Math::Mat4 skyboxCamera; // P * View with translation stripped, for skybox rendering
        Math::Mat4 lightCamera;
        Math::Vec3 lightDir;
        float pad0; // std140 alignment
        Math::Vec3 cameraPos;
        float pad1;
    };

    struct DrawData {
        Math::Mat4 model;
        Math::Mat3 normalMatrix;
    };
} // namespace Dodo
