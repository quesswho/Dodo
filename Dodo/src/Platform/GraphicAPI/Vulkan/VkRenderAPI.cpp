#include "VkRenderAPI.h"
#include "pch.h"

#include <backends/imgui_impl_vulkan.h>
#include <unordered_set>

namespace Dodo::Platform {

    VkRenderAPI::VkRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle) {}

    VkRenderAPI::~VkRenderAPI()
    {
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    RenderInitError VkRenderAPI::Init(const WindowProperties& winprop)
    {
        RenderInitError result = RenderInitError(RenderInitStatus::Success, "");

        m_Context.CreateContextImpl(m_Handle);

        // Preload Vulkan using volk
        if (volkInitialize() != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Glad: Unable to load Vulkan symbols!");
        }

        if (result = InitInstance(); result.status != RenderInitStatus::Success) return result;
        if (winprop.m_Settings.imgui) {
            if (result = InitImGui(); result.status != RenderInitStatus::Success) return result;
        }

        return result;
    }

    RenderInitError VkRenderAPI::InitInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dodo Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Get required context extensions
        std::vector<const char*> requiredExtensions = m_Context.GetExtensions();

        // Note: This might be useful for macOS, but I have no target to test it on
        // requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        // createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        // Get all available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::unordered_set<std::string> availableExtensions;
        DD_INFO("Available vulkan extensions: ");
        for (const auto& extension : extensions) {
            availableExtensions.insert(extension.extensionName);
            DD_INFO("{}", extension.extensionName);
        }

        // Check if all required extensions are available
        for (const char* required : requiredExtensions) {
            if (availableExtensions.find(required) == availableExtensions.end()) {
                return RenderInitError(RenderInitStatus::Failed,
                                       std::string("Required Vulkan extension not available: ") + required);
            }
        }

        // Add required extensions
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        createInfo.enabledLayerCount = 0;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);

        if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Failed to create Vulkan instance!");
        }
        return RenderInitError(RenderInitStatus::Success, "");
    }

    RenderInitError VkRenderAPI::InitImGui()
    {
        m_Context.InitializeImGui();
        // ImGui_ImplVulkan_Init();

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