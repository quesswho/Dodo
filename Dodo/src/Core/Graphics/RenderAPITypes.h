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
        Math::Mat4 cameraView;   // view matrix only (no projection), for cascade depth selection
        Math::Mat4 skyboxCamera; // P * View with translation stripped, for skybox rendering
        Math::Mat4 lightCamera;
        Math::Vec3 lightDir;
        float pad0; // std140 alignment
        Math::Vec3 cameraPos;
        float pad1;
    };

    struct CsmData {
        Math::Mat4 lightSpaceMatrices[4];
        float cascadeSplitDepths[4];
        int numCascades;
        float pad[3];
    };

    struct DrawData {
        Math::Mat4 model;
        Math::Mat3 normalMatrix;
        // Bindless texture handles (Vulkan only): indices into g_Textures[].
        // Slot mapping: [0]=albedo, [1]=roughness, [2]=normal, [3]=metallic,
        //               [4]=ao, [5]=spec, [6]=samplerIdx, [7]=unused
        uint32_t textureHandles[8] = {};
    };
} // namespace Dodo
