#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/RenderInitResult.h"

#include <Platform/WindowAPI/NativeWindowHandle.h>
#ifdef DD_API_WIN32
#include "WGLContext.h"
using VulkanContext = Dodo::Platform::VulkanWGLContext;
#elif DD_API_GLFW
#include "GLFWContext.h"
using VulkanContext = Dodo::Platform::VulkanGLFWContext;
#endif

namespace Dodo::Platform {

    class VulkanRenderAPI {
      public:
        VulkanRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~VulkanRenderAPI();
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

        inline void DepthTest(bool depthtest) const;
        inline void StencilTest(bool stenciltest) const;
        void Blending(bool blending) const;
        void Culling(bool cull, bool backface = true);

        inline const char* GetAPIName() const { return "Vulkan"; }
        int CurrentVRamUsage() const;

        VulkanContext m_Context;

      private:
        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform