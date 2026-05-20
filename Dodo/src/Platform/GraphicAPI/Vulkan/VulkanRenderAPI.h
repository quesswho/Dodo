#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/GpuTimings.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/RenderAPITypes.h"
#include "Core/Data/TextureLoader.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorAllocator.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorLayoutCache.h"
#include "Platform/GraphicAPI/Vulkan/VulkanFrameBufferedDescriptorSet.h"
#include "Platform/GraphicAPI/Vulkan/VulkanGpuPass.h"
#include "Platform/GraphicAPI/Vulkan/VulkanGpuPassQueue.h"

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
#include <string>
#include <unordered_map>
#include <vector>

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
        VkPhysicalDeviceMemoryProperties memoryProperties;
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
        VkFence inFlightFence;
    };

    class VulkanRenderAPI {
      public:
        VulkanRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~VulkanRenderAPI();
        RenderInitError Init(const WindowProperties& winprop);

        void WaitIdle() const;
        void Begin();
        void End();

        void ClearColor(float r, float g, float b) const;
        void Viewport(uint width, uint height) const;
        void BindCubeMap(uint slot, Ref<CubeMap> cubemap);
        void BindTexture(uint slot, Ref<Texture> texture);
        void BindTextureSampler(uint slot, Ref<TextureSampler> sampler);
        void BindFrameBufferTexture(uint slot, Ref<FrameBuffer> framebuffer);
        void* GetFrameBufferImGuiTextureID(Ref<FrameBuffer> framebuffer);
        void BindVertexBuffer(const Ref<VertexBuffer>& vb);
        void BindIndexBuffer(const Ref<IndexBuffer>& ib);
        void BindPipeline(Ref<Pipeline> pipeline);
        void SetMaterialDescriptorSet(VulkanFrameBufferedDescriptorSet& matSet) { m_BoundMaterialSet = &matSet; }
        void PushConstants(const void* data, size_t size);
        void SetFrameData(const Dodo::FrameData& data);
        void SetDrawData(const DrawData& data);
        void SetCSMData(const CsmData& data);
        void DrawIndices(uint count);
        void DrawIndicesInstanced(uint count, uint instanceCount);
        void DrawArray(uint count);
        void DefaultFrameBuffer();
        void BindFrameBuffer(Ref<FrameBuffer> framebuffer);
        void SetViewport(uint width, uint height);
        void SetViewport(uint width, uint height, uint posX, uint posY);

        // Factory methods, these are needed because we need context info
        Ref<Pipeline> CreatePipeline(const PipelineDesc& desc, AssetManager& assets);
        Ref<VertexBuffer> CreateVertexBuffer(const float* vertices, uint size, const BufferProperties& prop);
        Ref<IndexBuffer> CreateIndexBuffer(const uint* indices, uint count);
        Ref<Texture> CreateTexture(const uchar* data, const TextureProperties& prop);
        Ref<Texture> CreateTexture(const TextureData& data) { return CreateTexture(data.pixels.data(), data.props); }
        Ref<TextureSampler> CreateSampler(const SamplerProperties& prop);
        Ref<CubeMap> CreateCubeMap(const CubeMapData& data);
        Ref<CubeMap> CreateCubeMapFromEquirectangular(Ref<Texture> equirect, uint faceSize, AssetManager& assets);
        Ref<CubeMap> CreateIrradianceMap(Ref<CubeMap> envMap, uint faceSize, AssetManager& assets);
        Ref<FrameBuffer> CreateFrameBuffer(const FrameBufferProperties& props);

        // Batched async texture upload. CreateTexture/CreateCubeMap record into a shared command
        // buffer that is submitted once via SubmitTextureBatch. PollTextureBatch returns true when
        // the fence has signaled (or no batch is pending), at which point staging buffers are freed.
        void SubmitTextureBatch();
        bool PollTextureBatch();

        // One-shot GPU computation passes. SubmitGpuPass records and queues a pass asynchronously.
        // PollGpuPasses checks completion each frame and calls Finalize() on finished passes.
        // WaitGpuPasses blocks until all pending passes are finalized (use for chained passes).
        void SubmitGpuPass(std::unique_ptr<VulkanGpuPass> pass);
        void PollGpuPasses();
        void WaitGpuPasses();

        void BeginTimestamp(Dodo::GpuTimestampSlot slot);
        void EndTimestamp(Dodo::GpuTimestampSlot slot);

        inline const char* GetAPIName() const { return "Vulkan"; }
        int CurrentVRamUsage() const;

        void ImGuiNewFrame() const;
        void ImGuiEndFrame();

        VulkanContext m_Context;

        std::string m_GPUInfo;
        int m_VramKbs;

        uint m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY;

        bool m_CullingDefault;

        Dodo::GpuTimings m_GpuTimings;

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
        RenderInitError InitDescriptors();
        RenderInitError InitTimestampPools();
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
        bool IsBetterSurfaceFormat(VkSurfaceFormatKHR candidate, VkSurfaceFormatKHR current) const;
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
        typedef struct VmaAllocation_T* VmaAllocation;
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

        struct ImGuiFrameBufferEntry {
            VkDescriptorSet set;
            VkExtent2D extent;
        };
        std::unordered_map<VulkanFrameBuffer*, ImGuiFrameBufferEntry> m_ImGuiFrameBufferEntries;

        // Pending texture/sampler state (bound before each draw)
        static constexpr int maxTextureSlots = 8;
        VkImageView m_PendingImageViews[maxTextureSlots] = {};
        bool m_PendingIsCubeMap[maxTextureSlots] = {};
        bool m_PendingIsDepth[maxTextureSlots] = {};
        VkSampler m_PendingSamplers[maxTextureSlots] = {};

        // Fallback 1x1 resources used for set-1 bindings with no pending image
        VkImage m_DummyImage = VK_NULL_HANDLE;
        VmaAllocation m_DummyAllocation = nullptr;
        VkImageView m_DummyImageView = VK_NULL_HANDLE;
        VkSampler m_DummySampler = VK_NULL_HANDLE;

        // Descriptor infrastructure shared across all pipelines
        static constexpr int maxFramesInFlight = 2;
        std::unique_ptr<VulkanDescriptorLayoutCache> m_LayoutCache;
        std::unique_ptr<VulkanDescriptorAllocator> m_DescriptorAllocator;

        // Global set 0 (FrameData + CsmData): single descriptor set per frame, bound once at
        // the start of each command buffer. All pipeline layouts use the same set 0 layout so
        // the binding is never invalidated by pipeline switches within a frame.
        VkDescriptorSetLayout m_GlobalSet0Layout = VK_NULL_HANDLE;
        VkPipelineLayout m_GlobalFrameLayout = VK_NULL_HANDLE;
        std::array<VulkanDescriptorSet, maxFramesInFlight> m_GlobalSet0{};

        // Global set 2 (ModelData dynamic UBO): shared across all pipelines. All pipelines that
        // declare set 2 point to the same UBO buffers, so one descriptor set per frame suffices.
        VkDescriptorSetLayout m_GlobalSet2Layout = VK_NULL_HANDLE;
        std::array<VulkanDescriptorSet, maxFramesInFlight> m_GlobalSet2{};

        // GPU-side layout for ModelData cbuffer (float4x4 model + float4x4 normal = 128 bytes)
        struct GPUModelData {
            float model[16];  // Mat4
            float normal[16]; // Mat4 (Mat3 zero-padded into 4th column)
        };

        // Persistently-mapped UBOs for FrameData (one per frame)
        struct MappedBuffer {
            VkBuffer buffer = VK_NULL_HANDLE;
            VmaAllocation allocation = nullptr;
            void* mapped = nullptr;
        };
        std::array<MappedBuffer, maxFramesInFlight> m_FrameDataUBOs{};
        std::array<MappedBuffer, maxFramesInFlight> m_CsmDataUBOs{};

        // Dynamic UBO ring buffer for per-draw ModelData (one large buffer per frame)
        static constexpr uint32_t maxDrawsPerFrame = 1024;
        std::array<MappedBuffer, maxFramesInFlight> m_ModelDataUBOs{};
        uint32_t m_ModelUBOSlotSize = 0; // sizeof(GPUModelData) aligned to minUniformBufferOffsetAlignment
        uint32_t m_ModelUBOCursor = 0;   // next free slot index within the current frame
        uint32_t m_LastModelOffset = 0;  // byte offset written by the most recent SetDrawData

        // Current pipeline reference (needed for set-1 layout when binding textures)
        class VulkanPipeline* m_BoundPipelinePtr = nullptr;
        VulkanFrameBufferedDescriptorSet* m_BoundMaterialSet = nullptr;
        class VulkanFrameBuffer* m_BoundFrameBuffer = nullptr;
        bool m_IsRendering = false;

        VulkanGpuPassContext MakeGpuPassContext();
        std::unique_ptr<VulkanGpuPassQueue> m_GpuPassQueue;

        // Upload batch: one command buffer shared across all CreateTexture/CreateCubeMap calls per frame.
        // Submitted once at the end of FlushStagingQueue; fence polled next frame to free staging memory.
        VkCommandBuffer m_UploadCmdBuf = VK_NULL_HANDLE;
        VkFence m_UploadFence = VK_NULL_HANDLE;
        bool m_UploadBatchActive = false;
        bool m_UploadFencePending = false;
        std::vector<Ref<Texture>> m_UploadBatchTextures;
        std::vector<Ref<CubeMap>> m_UploadBatchCubeMaps;

        // Frame stuff
        std::array<FrameData, maxFramesInFlight> m_Frames;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        uint m_CurrentFrame = 0;
        uint32_t m_CurrentImageIndex = 0;
        uint32_t m_FrameEpoch = 0;

        static constexpr uint32_t maxTimestampQueries = static_cast<uint32_t>(Dodo::GpuTimestampSlot::Count) * 2;
        std::array<VkQueryPool, maxFramesInFlight> m_TimestampPools{};
        float m_TimestampPeriodNs = 1.0f;
        bool m_TimestampsSupported = false;
        void ReadTimestamps();

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