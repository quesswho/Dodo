#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/RenderAPITypes.h"

#include "Platform/WindowAPI/NativeWindowHandle.h"
#ifdef DD_API_WIN32
#include "VulkanWGLContext.h"
using VulkanContext = Dodo::Platform::VulkanWGLContext;
#elif defined(DD_API_GLFW)
#include "VulkanGLFWContext.h"
using VulkanContext = Dodo::Platform::VulkanGLFWContext;
#endif

#define DODO_VULKAN_VERSION VK_API_VERSION_1_3

#include <array>
#include <optional>

namespace Dodo {
    class AssetManager;
}

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

    struct FrameData {
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
    };

    class VulkanRenderAPI {
      public:
        VulkanRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~VulkanRenderAPI();
        RenderInitError Init(const WindowProperties& winprop);

        void Begin();
        void End();

        void ClearColor(float r, float g, float b) const;
        void Viewport(uint width, uint height) const;
        void BindCubeMap(uint slot, Ref<CubeMap> cubemap);
        void BindTexture(uint slot, Ref<Texture> texture);
        void BindTextureSampler(uint slot, Ref<TextureSampler> sampler);
        void BindFrameBufferTexture(uint slot, Ref<FrameBuffer> framebuffer);
        void BindVertexBuffer(const Ref<VertexBuffer>& vb);
        void BindIndexBuffer(const Ref<IndexBuffer>& ib);
        void BindPipeline(Ref<Pipeline> pipeline);
        void PushConstants(const void* data, size_t size);
        void SetFrameData(const Dodo::FrameData& data);
        void SetDrawData(const DrawData& data);
        void DrawIndexed(const Ref<VertexBuffer>& va);
        void DrawIndices(uint count) const;
        void DrawArray(uint count) const;
        void DefaultFrameBuffer() const;
        void SetViewport(uint width, uint height);
        void SetViewport(uint width, uint height, uint posX, uint posY);

        // Factory methods, these are needed because we need context info
        Ref<Pipeline> CreatePipeline(const PipelineDesc& desc, AssetManager& assets);
        Ref<VertexBuffer> CreateVertexBuffer(const float* vertices, uint size, const BufferProperties& prop);
        Ref<IndexBuffer> CreateIndexBuffer(const uint* indices, uint count);
        Ref<Texture> CreateTexture(uchar* data, const TextureProperties& prop);
        Ref<TextureSampler> CreateSampler(const SamplerProperties& prop);

        inline const char* GetAPIName() const { return "Vulkan"; }
        int CurrentVRamUsage() const;

        void ImGuiNewFrame() const;
        void ImGuiEndFrame();

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
        RenderInitError InitVMA();
        RenderInitError CreateSwapChain(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
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

        void RecreateSwapChain();

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

        // Forward-declared to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION is defined
        typedef struct VmaAllocator_T* VmaAllocator;
        VmaAllocator m_VmaAllocator = nullptr;

        VkInstance m_VkInstance;
        VkDevice m_Device;
        VkPhysicalDevice m_PhysicalDevice;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
        VkSwapchainKHR m_SwapChain;
        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;
        VkCommandPool m_CommandPool;
        VkPipeline m_BoundPipeline;
        VkPipelineLayout m_BoundPipelineLayout = VK_NULL_HANDLE;

        // Note: This is temporary for now
        static constexpr int maxTextureSlots = 3;
        VkImageView m_PendingImageViews[maxTextureSlots] = {};
        VkSampler m_PendingSamplers[maxTextureSlots] = {};

        // Frame stuff
        static constexpr int maxFramesInFlight = 2;
        std::array<FrameData, maxFramesInFlight> m_Frames;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        uint m_CurrentFrame = 0;
        uint32_t m_CurrentImageIndex = 0;

        VkDescriptorPool m_ImGuiDescriptorPool;
        bool m_ImGuiActive = false;

        bool m_SwapChainNeedsRecreation = false;
        bool m_EnableValidationLayers;
        std::vector<const char*> m_ValidationLayers;
        std::vector<const char*> m_DeviceExtensions;

        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform