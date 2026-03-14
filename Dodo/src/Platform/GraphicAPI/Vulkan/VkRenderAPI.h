#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
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

        void Begin() const;
        void End();

        
        void ClearColor(float r, float g, float b) const;
        void Viewport(uint width, uint height) const;
        void BindCubeMap(uint slot, Ref<CubeMap> cubemap);
        void BindTexture(uint slot, Ref<Texture> texture);
        void BindTextureSampler(uint slot, Ref<TextureSampler> sampler);
        void DrawIndices(uint count) const;
        void DrawArray(uint count) const;
        void DefaultFrameBuffer() const;
        void ResizeDefaultViewport(uint width, uint height);
        void ResizeDefaultViewport(uint width, uint height, uint posX, uint posY);
        
        void DepthComparisonMethod(DepthComparisonMethod method) const;
        void DepthTest(bool depthtest) const;
        void StencilTest(bool stenciltest) const;
        void Blending(bool blending) const;
        void Culling(bool cull, bool backface = true);
        
        inline const char* GetAPIName() const { return "Vulkan"; }
        int CurrentVRamUsage() const;

        void ImGuiNewFrame() const;
        void ImGuiEndFrame() const;

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