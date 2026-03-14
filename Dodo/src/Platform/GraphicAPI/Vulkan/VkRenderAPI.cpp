#include "VkRenderAPI.h"
#include "pch.h"

#include <backends/imgui_impl_vulkan.h>

namespace Dodo { namespace Platform {

    VkRenderAPI::VkRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle) {}

    VkRenderAPI::~VkRenderAPI() {}

    RenderInitError VkRenderAPI::Init(const WindowProperties& winprop)
    {
        m_Context.CreateContextImpl(m_Handle); // Run glad loader

        return RenderInitError(RenderInitStatus::Success, "");
    }

    void VkRenderAPI::DefaultFrameBuffer() const {}

    void VkRenderAPI::Blending(bool blending) const {}

    void VkRenderAPI::Culling(bool cull, bool backface) {}

    void VkRenderAPI::ResizeDefaultViewport(uint width, uint height) {}

    void VkRenderAPI::ResizeDefaultViewport(uint width, uint height, uint posX, uint posY) {}

    void VkRenderAPI::ImGuiNewFrame() const
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VkRenderAPI::ImGuiEndFrame() const
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
    }
}} // namespace Dodo::Platform