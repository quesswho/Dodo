#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/RenderAPITypes.h"
#include "Core/Graphics/RenderInitResult.h"

#include "Platform/WindowAPI/NativeWindowHandle.h"
#ifdef DD_API_WIN32
#include "VkWGLContext.h"
using VulkanContext = Dodo::Platform::VulkanWGLContext;
#elif defined(DD_API_GLFW)
#include "VkGLFWContext.h"
using VulkanContext = Dodo::Platform::VulkanGLFWContext;
#endif

namespace Dodo::Platform {

    class VkRenderAPI {
      public:
        VkRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~VkRenderAPI();
        RenderInitError Init(const WindowProperties& winprop);

        inline void Begin() const;
        inline void End();

        void ImGuiNewFrame() const;
        void ImGuiEndFrame() const;

        inline void ClearColor(float r, float g, float b) const;
        inline void Viewport(uint width, uint height) const;
        inline void DrawIndices(uint count) const;
        inline void DrawArray(uint count) const;
        void DefaultFrameBuffer() const;
        void ResizeDefaultViewport(uint width, uint height);
        void ResizeDefaultViewport(uint width, uint height, uint posX, uint posY);

        inline void DepthComparisonMethod(DepthComparisonMethod method) const;
        inline void DepthTest(bool depthtest) const;
        inline void StencilTest(bool stenciltest) const;
        void Blending(bool blending) const;
        void Culling(bool cull, bool backface = true);

        inline const char* GetAPIName() const { return "Vulkan"; }
        int CurrentVRamUsage() const;

        VulkanContext m_Context;

        std::string m_GPUInfo;
        int m_VramKbs;

        uint m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY;

        bool m_CullingDefault;

      private:
        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform