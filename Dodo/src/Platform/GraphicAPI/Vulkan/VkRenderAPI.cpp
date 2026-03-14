#include "VkRenderAPI.h"
#include "pch.h"

#include <backends/imgui_impl_vulkan.h>

namespace Dodo::Platform {

    VkRenderAPI::VkRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle) {}

    VkRenderAPI::~VkRenderAPI() {}

    RenderInitError VkRenderAPI::Init(const WindowProperties& winprop)
    {
        m_Context.CreateContextImpl(m_Handle); // Run glad loader

        return RenderInitError(RenderInitStatus::Success, "");
    }

    void VkRenderAPI::Begin() const {}
    void VkRenderAPI::End() {}

    void VkRenderAPI::ClearColor(float r, float g, float b) const {}
    void VkRenderAPI::Viewport(uint width, uint height) const {}
    void VkRenderAPI::BindCubeMap(uint slot, Ref<CubeMap> cubemap) {}
    void VkRenderAPI::BindTexture(uint slot, Ref<Texture> texture) {}
    void VkRenderAPI::BindTextureSampler(uint slot, Ref<TextureSampler> sampler) {}
    void VkRenderAPI::DrawIndices(uint count) const {}
    void VkRenderAPI::DrawArray(uint count) const {}

    void VkRenderAPI::DefaultFrameBuffer() const {}
    void VkRenderAPI::ResizeDefaultViewport(uint width, uint height) {}
    void VkRenderAPI::ResizeDefaultViewport(uint width, uint height, uint posX, uint posY) {}

    void VkRenderAPI::DepthComparisonMethod(Dodo::DepthComparisonMethod method) const {}
    void VkRenderAPI::DepthTest(bool depthtest) const {}
    void VkRenderAPI::StencilTest(bool stenciltest) const {}
    void VkRenderAPI::Blending(bool blending) const {}
    void VkRenderAPI::Culling(bool cull, bool backface) {}
    
    void VkRenderAPI::ImGuiNewFrame() const
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VkRenderAPI::ImGuiEndFrame() const
    {
        // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
    }
} // namespace Dodo::Platform