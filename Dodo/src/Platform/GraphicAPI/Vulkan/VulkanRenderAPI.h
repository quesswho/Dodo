#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/RenderAPITypes.h"

#include "Platform/WindowAPI/NativeWindowHandle.h"
#ifdef DD_API_WIN32
#include "VulkanWGLContext.h"
using VulkanContext = Dodo::Platform::VulkanWGLContext;
#elif defined(DD_API_GLFW)
#include "VulkanGLFWContext.h"
using VulkanContext = Dodo::Platform::VulkanGLFWContext;
#endif

#include <optional>

namespace Dodo::Platform {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct PhyisicalDeviceInfo {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        QueueFamilyIndices indices;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanRenderAPI {
      public:
        VulkanRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~VulkanRenderAPI();
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
        RenderInitError InitInstance();
        RenderInitError SetupDebug();
        RenderInitError PickPhysicalDevice();
        RenderInitError InitDevice();
        RenderInitError CreateSwapChain();
        RenderInitError CreateImageViews();
        RenderInitError CreateCommandPool();
        RenderInitError CreateCommandBuffer();
        RenderInitError CreateSyncObjects();
        RenderInitError InitImGui();

        bool IsDeviceBetter(PhyisicalDeviceInfo bestDevice, PhyisicalDeviceInfo device);
        bool IsDeviceSuitable(PhyisicalDeviceInfo device);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        std::vector<const char*> GetRequiredExtensions();
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              VkDebugUtilsMessengerEXT* pDebugMessenger);
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks* pAllocator);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);

        VkInstance m_VkInstance;
        VkDevice m_Device;
        VkPhysicalDevice m_PhysicalDevice;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
        VkSwapchainKHR m_SwapChain;
        VkQueue m_PresentQueue;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;
        VkCommandPool m_CommandPool;
        VkCommandBuffer m_CommandBuffer;

        // Frame synchronization
        VkSemaphore m_ImageAvailableSemaphore;
        VkSemaphore m_RenderFinishedSemaphore;
        VkFence m_InFlightFence;

        VkDescriptorPool m_ImGuiDescriptorPool;

        bool m_EnableValidationLayers;
        std::vector<const char*> m_ValidationLayers;
        std::vector<const char*> m_DeviceExtensions;

        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform