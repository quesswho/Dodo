#include "VulkanRenderAPI.h"
#include "pch.h"

#include "Core/Application/Application.h"

#include <backends/imgui_impl_vulkan.h>

namespace Dodo { namespace Platform {

    VulkanRenderAPI::VulkanRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle) {}

    VulkanRenderAPI::~VulkanRenderAPI() {}

    RenderInitError VulkanRenderAPI::Init(const WindowProperties& winprop)
    {
        m_Context.CreateContextImpl(m_Handle); // Run glad loader

        return RenderInitError(RenderInitStatus::Success, "");
    }

    void VulkanRenderAPI::DefaultFrameBuffer() const {}

    void VulkanRenderAPI::Blending(bool blending) const {}

    void VulkanRenderAPI::Culling(bool cull, bool backface) {}

    void VulkanRenderAPI::ResizeDefaultViewport(uint width, uint height) {}

    void VulkanRenderAPI::ResizeDefaultViewport(uint width, uint height, uint posX, uint posY) {}

    void VulkanRenderAPI::ImGuiNewFrame() const { ImGui_ImplVulkan_NewFrame(); }

    void VulkanRenderAPI::ImGuiEndFrame() const { ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData()); }
}} // namespace Dodo::Platform