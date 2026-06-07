#include "VulkanRenderAPI.h"

#include "Core/Data/AssetManager.h"
#include "VulkanBuffer.h"
#include "VulkanCubeMap.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGpuPassQueue.h"
#include "VulkanPipeline.h"
#include "VulkanSampler.h"
#include "VulkanTexture.h"
#include "Passes/VulkanEquirectangularPass.h"
#include "Passes/VulkanCubemapConvolutionPass.h"

#include <backends/imgui_impl_vulkan.h>
#include <unordered_set>

#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    VulkanRenderAPI::VulkanRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle)
    {
#ifdef DD_DEBUG
        m_EnableValidationLayers = true;
        m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#else
        m_EnableValidationLayers = false;
#endif
    }

    VulkanRenderAPI::~VulkanRenderAPI()
    {
        // TODO: I am unsure if we need to wait for both, but should be for good measure
        vkQueueWaitIdle(m_GraphicsQueue);
        vkQueueWaitIdle(m_PresentQueue);

        // Finish all pending GPU passes before destroying the command pool they use.
        m_GpuPassQueue.reset();

        for (const auto& semaphore : m_RenderFinishedSemaphores) {
            vkDestroySemaphore(m_Device, semaphore, nullptr);
        }

        for (int i = 0; i < maxFramesInFlight; i++) {
            vkDestroySemaphore(m_Device, m_Frames[i].imageAvailableSemaphore, nullptr);
            vkDestroyFence(m_Device, m_Frames[i].inFlightFence, nullptr);
        }

        if (m_UploadFence) vkDestroyFence(m_Device, m_UploadFence, nullptr);

        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

        // Descriptor infrastructure
        for (int i = 0; i < maxFramesInFlight; i++) {
            if (m_FrameDataUBOs[i].buffer)
                vmaDestroyBuffer(m_VmaAllocator, m_FrameDataUBOs[i].buffer, m_FrameDataUBOs[i].allocation);
            if (m_ModelDataUBOs[i].buffer)
                vmaDestroyBuffer(m_VmaAllocator, m_ModelDataUBOs[i].buffer, m_ModelDataUBOs[i].allocation);
            if (m_CsmDataUBOs[i].buffer)
                vmaDestroyBuffer(m_VmaAllocator, m_CsmDataUBOs[i].buffer, m_CsmDataUBOs[i].allocation);
        }
        if (m_GlobalFrameLayout) vkDestroyPipelineLayout(m_Device, m_GlobalFrameLayout, nullptr);
        if (m_GlobalSet1Layout) vkDestroyDescriptorSetLayout(m_Device, m_GlobalSet1Layout, nullptr);
        if (m_GlobalSet0Layout) vkDestroyDescriptorSetLayout(m_Device, m_GlobalSet0Layout, nullptr);
        // Free all registered named samplers (default 0-5 plus any user-added)
        for (auto s : m_RegisteredSamplers)
            if (s) vkDestroySampler(m_Device, s, nullptr);
        m_BindlessAllocator.reset();
        m_LayoutCache.reset();
        m_DescriptorAllocator.reset();
        if (m_DummySampler) vkDestroySampler(m_Device, m_DummySampler, nullptr);
        if (m_DummyImageView) vkDestroyImageView(m_Device, m_DummyImageView, nullptr);
        if (m_DummyImage) vmaDestroyImage(m_VmaAllocator, m_DummyImage, m_DummyAllocation);

        for (auto pool : m_TimestampPools)
            if (pool != VK_NULL_HANDLE) vkDestroyQueryPool(m_Device, pool, nullptr);

        if (m_ImGuiActive) {
            for (auto& [fb, entry] : m_ImGuiFrameBufferEntries)
                ImGui_ImplVulkan_RemoveTexture(entry.set);
            m_ImGuiFrameBufferEntries.clear();
            for (auto& [tex, set] : m_ImGuiTextureEntries)
                ImGui_ImplVulkan_RemoveTexture(set);
            m_ImGuiTextureEntries.clear();
            ImGui_ImplVulkan_Shutdown();
            vkDestroyDescriptorPool(m_Device, m_ImGuiDescriptorPool, nullptr);
        }

        for (auto imageView : m_SwapChainImageViews) {
            vkDestroyImageView(m_Device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
        vmaDestroyAllocator(m_VmaAllocator);
        vkDestroyDevice(m_Device, nullptr);
        if (m_EnableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    void VulkanRenderAPI::WaitIdle() const
    {
        vkDeviceWaitIdle(m_Device);
    }

    /**
     * Initializes the Vulkan instance, picks a physical device, creates a logical device and initializes ImGui if
     * enabled in the window properties.
     */
    RenderInitError VulkanRenderAPI::Init(const WindowProperties& winprop)
    {
        RenderInitError result = RenderInitError(RenderInitStatus::Success);

        m_Context.CreateContextImpl(m_Handle);

        // Preload Vulkan using volk
        if (volkInitialize() != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Volk: Unable to load Vulkan symbols!");
        }

        // Lambda to simplify error checking
        auto Try = [&](RenderInitError res) -> bool {
            result = res;
            return result.status != RenderInitStatus::Success;
        };

        if (Try(InitInstance())) return result;
        if (m_EnableValidationLayers)
            if (Try(SetupDebug())) return result;
        if (!m_Context.CreateSurface(m_VkInstance, &m_Surface)) {
            return RenderInitError(RenderInitStatus::Failed, "Failed to create Vulkan surface!");
        }
        if (Try(PickPhysicalDevice())) return result;
        if (Try(InitDevice())) return result;
        if (Try(InitVMA())) return result;
        if (Try(CreateSwapChain())) return result;
        if (Try(CreateImageViews())) return result;
        if (Try(CreateCommandPool())) return result;
        if (Try(CreateCommandBuffer())) return result;
        if (Try(CreateSyncObjects())) return result;
        if (Try(InitDescriptors())) return result;
        if (Try(InitTimestampPools())) return result;

        m_GpuPassQueue = std::make_unique<VulkanGpuPassQueue>(MakeGpuPassContext());

        if (winprop.m_Settings.imgui)
            if (Try(InitImGui())) return result;

        return result;
    }

    /**
     * Initializes the Vulkan instance and initialize volk
     */
    RenderInitError VulkanRenderAPI::InitInstance()
    {
        if (m_EnableValidationLayers && !CheckValidationLayerSupport())
            return RenderInitError(RenderInitStatus::Failed, "validation layers requested, but not available!");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "Dodo Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = DODO_VULKAN_VERSION;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

// Add portability enumeration flag for platforms that require it (e.g. macOS with MoltenVK)
#ifdef DD_MACOS
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        // Get required context extensions
        std::vector<const char*> requiredExtensions = GetRequiredExtensions();

        // Get all available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::unordered_set<std::string> availableExtensions;
        // DD_INFO("Available vulkan extensions: ");
        for (const auto& extension : extensions) {
            availableExtensions.insert(extension.extensionName);
            // DD_INFO("{}", extension.extensionName);
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

        if (m_EnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Failed to create Vulkan instance!");
        }

        // Load all required Vulkan entrypoints, including all extensions
        volkLoadInstance(m_VkInstance);

        return RenderInitError(RenderInitStatus::Success);
    }

    /**
     * Sets up the debug messenger callback for Vulkan validation layers
     */
    RenderInitError VulkanRenderAPI::SetupDebug()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to set up debug messenger!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    /**
     * Selects the best physical device that supports the required features and extensions
     */
    RenderInitError VulkanRenderAPI::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            return RenderInitError(RenderInitStatus::Failed, "Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

        m_PhysicalDevice = VK_NULL_HANDLE;

        PhyisicalDeviceInfo bestDeviceInfo;
        bestDeviceInfo.device = VK_NULL_HANDLE;
        for (const auto& device : devices) {
            PhyisicalDeviceInfo deviceInfo;
            VkPhysicalDeviceProperties2 deviceProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            VkPhysicalDeviceFeatures2 deviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
            vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

            deviceInfo.device = device;
            deviceInfo.properties = deviceProperties2.properties;
            deviceInfo.features = deviceFeatures2.features;
            VkPhysicalDeviceMemoryProperties2 memProps2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
            vkGetPhysicalDeviceMemoryProperties2(device, &memProps2);
            deviceInfo.memoryProperties = memProps2.memoryProperties;
            deviceInfo.indices = FindQueueFamilies(device);
            if (!IsDeviceSuitable(deviceInfo)) {
                DD_INFO("Device {} is not suitable", deviceInfo.properties.deviceName);
                continue;
            }
            if (IsDeviceBetter(bestDeviceInfo, deviceInfo)) {
                bestDeviceInfo = deviceInfo;
                m_PhysicalDevice = device;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            return RenderInitError(RenderInitStatus::Failed, "failed to find a suitable GPU!");
        }

        DD_INFO("Selected Device: {}", bestDeviceInfo.properties.deviceName);

        return RenderInitError(RenderInitStatus::Success);
    }

    /**
     * Creates a logical device from the selected physical device and retrieves the graphics and present queues
     */
    RenderInitError VulkanRenderAPI::InitDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        // Normally graphics and present queues are the same, but they can be different on some platforms.
        std::unordered_set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                                            indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Fetch and check required device extensions
        std::vector<const char*> deviceExtensions = GetRequiredDeviceExtensions(m_PhysicalDevice);
        if (!CheckDeviceExtensionSupport(m_PhysicalDevice, deviceExtensions)) {
            return RenderInitError(RenderInitStatus::Failed, "Physical device does not support required extensions!");
        }

        VkPhysicalDeviceVulkan13Features vulkan13Features{};
        vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13Features.dynamicRendering = VK_TRUE;
        vulkan13Features.synchronization2 = VK_TRUE;
        vulkan13Features.shaderDemoteToHelperInvocation = VK_TRUE;

        VkPhysicalDeviceVulkan12Features vulkan12Features{};
        vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12Features.hostQueryReset = VK_TRUE;
        vulkan12Features.shaderOutputLayer = VK_TRUE;
        vulkan12Features.descriptorBindingPartiallyBound               = VK_TRUE;
        vulkan12Features.descriptorBindingUpdateUnusedWhilePending     = VK_TRUE;
        vulkan12Features.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
        vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        vulkan12Features.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
        vulkan12Features.runtimeDescriptorArray                        = VK_TRUE;
        vulkan12Features.pNext = &vulkan13Features;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.tessellationShader = VK_TRUE;
        deviceFeatures.depthClamp = VK_TRUE;
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &vulkan12Features;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to create logical device!");
        }

        volkLoadDevice(m_Device);

        vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

        return RenderInitError(RenderInitStatus::Success);
    }

    /**
     * Initialize Vulkan Memory Allocator library
     */
    RenderInitError VulkanRenderAPI::InitVMA()
    {
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
        allocatorCreateInfo.device = m_Device;
        allocatorCreateInfo.instance = m_VkInstance;
        allocatorCreateInfo.vulkanApiVersion = DODO_VULKAN_VERSION;
        allocatorCreateInfo.flags = 0;

        VmaVulkanFunctions vulkanFunctions;
        VkResult res = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
        if (res != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "VMA: Failed to import Vulkan functions from Volk!");
        }

        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        res = vmaCreateAllocator(&allocatorCreateInfo, &m_VmaAllocator);
        if (res != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "VMA: Failed to create VMA allocator!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::CreateSwapChain(VkSwapchainKHR oldSwapchain)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t minCount = swapChainSupport.capabilities.minImageCount;
        uint32_t maxCount = swapChainSupport.capabilities.maxImageCount > 0
                                ? swapChainSupport.capabilities.maxImageCount
                                : (std::numeric_limits<uint32_t>::max)();
        // We prefer triple buffer, then double buffer
        uint32_t imageCount = (3 <= maxCount) ? (std::max)(3u, minCount) : maxCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;

        // This means we draw directly to the images, with framebuffers we would use VK_IMAGE_USAGE_TRANSFER_DST_BIT
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            // TODO: Data needs to be shared across queue families, needs some additional setup here
            // We might need hardware to properly test whenever we have different queue families
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        // Specify no transformations such as 90 degree rotation or flipping
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        // We might want to change this for alpha textures, but not clear
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE; // Disallow reading pixel in non-active buffers

        createInfo.oldSwapchain = oldSwapchain;

        if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;
        m_ViewportWidth = extent.width;
        m_ViewportHeight = extent.height;

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::CreateImageViews()
    {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
                return RenderInitError(RenderInitStatus::Failed, "Failed to create image views!");
            }
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::CreateCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to create command pool!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::CreateCommandBuffer()
    {
        std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = maxFramesInFlight;

        if (vkAllocateCommandBuffers(m_Device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to allocate command buffers!");
        }

        for (int i = 0; i < maxFramesInFlight; i++) {
            m_Frames[i].commandBuffer = commandBuffers[i];
        }

        // Allocate the shared upload command buffer used for batched texture uploads
        VkCommandBufferAllocateInfo uploadAllocInfo{};
        uploadAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        uploadAllocInfo.commandPool = m_CommandPool;
        uploadAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        uploadAllocInfo.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(m_Device, &uploadAllocInfo, &m_UploadCmdBuf) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to allocate upload command buffer!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::CreateSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Sets the fence into signaled state so that we do not wait for
                                                        // the first frame which does not exist

        m_RenderFinishedSemaphores.resize(m_SwapChainImages.size());
        for (auto& sem : m_RenderFinishedSemaphores) {
            if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &sem) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "failed to create render finished semaphore!");
        }

        // Create a semaphore for each frame in flight to synchronize when rendering is finished, and a fence to
        // synchronize CPU-GPU
        for (int i = 0; i < maxFramesInFlight; i++) {
            if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frames[i].imageAvailableSemaphore) !=
                    VK_SUCCESS ||
                vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Frames[i].inFlightFence) != VK_SUCCESS) {
                return RenderInitError(RenderInitStatus::Failed, "failed to create semaphores!");
            }
        }

        // Upload fence starts unsignaled: no batch is pending at init time
        VkFenceCreateInfo uploadFenceInfo{};
        uploadFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if (vkCreateFence(m_Device, &uploadFenceInfo, nullptr, &m_UploadFence) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to create upload fence!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::InitDescriptors()
    {
        // Compute slot size for the dynamic ModelData UBO (must be aligned to device limit)
        VkPhysicalDeviceProperties2 devProps2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &devProps2);
        uint32_t alignment = (uint32_t)devProps2.properties.limits.minUniformBufferOffsetAlignment;
        uint32_t rawSize = (uint32_t)sizeof(GPUModelData);
        m_ModelUBOSlotSize = (rawSize + alignment - 1) & ~(alignment - 1);

        m_LayoutCache = std::make_unique<VulkanDescriptorLayoutCache>(m_Device);
        m_DescriptorAllocator = std::make_unique<VulkanDescriptorAllocator>(m_Device);
        m_BindlessAllocator = std::make_unique<VulkanDescriptorAllocator>(m_Device, /*updateAfterBind=*/true);

        // --- Create per-frame UBOs via VMA ---
        VkBufferCreateInfo bufCI{};
        bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_AUTO;
        allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        for (int i = 0; i < maxFramesInFlight; i++) {
            VmaAllocationInfo vmaInfo{};

            // FrameData UBO
            bufCI.size = sizeof(Dodo::FrameData);
            if (vmaCreateBuffer(m_VmaAllocator, &bufCI, &allocCI, &m_FrameDataUBOs[i].buffer,
                                &m_FrameDataUBOs[i].allocation, &vmaInfo) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create FrameData UBO!");
            m_FrameDataUBOs[i].mapped = vmaInfo.pMappedData;

            // ModelData dynamic UBO (ring buffer for up to maxDrawsPerFrame draws)
            bufCI.size = (VkDeviceSize)m_ModelUBOSlotSize * maxDrawsPerFrame;
            if (vmaCreateBuffer(m_VmaAllocator, &bufCI, &allocCI, &m_ModelDataUBOs[i].buffer,
                                &m_ModelDataUBOs[i].allocation, &vmaInfo) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create ModelData UBO!");
            m_ModelDataUBOs[i].mapped = vmaInfo.pMappedData;

            // CsmData UBO (per-frame CSM matrices and split depths)
            bufCI.size = sizeof(Dodo::CsmData);
            if (vmaCreateBuffer(m_VmaAllocator, &bufCI, &allocCI, &m_CsmDataUBOs[i].buffer,
                                &m_CsmDataUBOs[i].allocation, &vmaInfo) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create CsmData UBO!");
            m_CsmDataUBOs[i].mapped = vmaInfo.pMappedData;
        }

        // --- Create default named samplers (indices 0-5) ---
        // These are registered eagerly so shaders can rely on fixed indices.
        // Sampler 0: LINEAR_REPEAT
        {
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_LINEAR;
            s.minFilter = VK_FILTER_LINEAR;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            s.maxLod = VK_LOD_CLAMP_NONE;
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }
        // Sampler 1: LINEAR_CLAMP
        {
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_LINEAR;
            s.minFilter = VK_FILTER_LINEAR;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            s.maxLod = VK_LOD_CLAMP_NONE;
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }
        // Sampler 2: NEAREST_REPEAT
        {
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_NEAREST;
            s.minFilter = VK_FILTER_NEAREST;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            s.maxLod = VK_LOD_CLAMP_NONE;
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }
        // Sampler 3: NEAREST_CLAMP
        {
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_NEAREST;
            s.minFilter = VK_FILTER_NEAREST;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            s.maxLod = VK_LOD_CLAMP_NONE;
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }
        // Sampler 4: LINEAR_CLAMP_BORDER_WHITE (used for shadow map sampling)
        {
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_LINEAR;
            s.minFilter = VK_FILTER_LINEAR;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            s.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            s.maxLod = VK_LOD_CLAMP_NONE;
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }
        // Sampler 5: ANISO16_REPEAT (falls back to LINEAR_REPEAT if anisotropy not supported)
        {
            VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);
            const VkPhysicalDeviceFeatures& features = features2.features;
            VkSamplerCreateInfo s{};
            s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            s.magFilter = VK_FILTER_LINEAR;
            s.minFilter = VK_FILTER_LINEAR;
            s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            s.maxLod = VK_LOD_CLAMP_NONE;
            if (features.samplerAnisotropy) {
                s.anisotropyEnable = VK_TRUE;
                s.maxAnisotropy = 16.0f;
            }
            VkSampler vks = VK_NULL_HANDLE;
            vkCreateSampler(m_Device, &s, nullptr, &vks);
            RegisterSampler(vks);
        }

        // --- Create global Set 0 layout (6 bindings, UPDATE_AFTER_BIND for b4/b5/b6/b7) ---
        // Bindings: 0=FrameData UBO, 3=CsmData UBO, 4=sampler[32], 5=ShadowMap, 6=EnvironmentMap, 7=IrradianceMap
        {
            VkDescriptorSetLayoutBinding b0{};
            b0.binding = 0;
            b0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            b0.descriptorCount = 1;
            b0.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            VkDescriptorSetLayoutBinding b3{};
            b3.binding = 3;
            b3.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            b3.descriptorCount = 1;
            b3.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            VkDescriptorSetLayoutBinding b4{};
            b4.binding = 4;
            b4.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            b4.descriptorCount = k_BindlessMaxSamplers;
            b4.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            VkDescriptorSetLayoutBinding b5{};
            b5.binding = 5;
            b5.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            b5.descriptorCount = 1;
            b5.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding b6{};
            b6.binding = 6;
            b6.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            b6.descriptorCount = 1;
            b6.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding b7{};
            b7.binding = 7;
            b7.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            b7.descriptorCount = 1;
            b7.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding set0Bindings[] = {b0, b3, b4, b5, b6, b7};

            // Only b4/b5/b6/b7 need UPDATE_AFTER_BIND (shadow/cubemaps updated mid-frame).
            // UBOs (b0, b3) must not use this flag without descriptorBindingUniformBufferUpdateAfterBind.
            VkDescriptorBindingFlags bindingFlags[6] = {
                0,
                0,
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            };
            VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{};
            flagsCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            flagsCI.bindingCount = 6;
            flagsCI.pBindingFlags = bindingFlags;

            VkDescriptorSetLayoutCreateInfo set0LayoutCI{};
            set0LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            set0LayoutCI.pNext = &flagsCI;
            set0LayoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            set0LayoutCI.bindingCount = (uint32_t)std::size(set0Bindings);
            set0LayoutCI.pBindings = set0Bindings;
            vkCreateDescriptorSetLayout(m_Device, &set0LayoutCI, nullptr, &m_GlobalSet0Layout);
        }

        // --- Create global Set 1 layout (bindless 2D texture array) ---
        {
            VkDescriptorSetLayoutBinding b1{};
            b1.binding = 0;
            b1.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            b1.descriptorCount = k_BindlessMaxTextures;
            b1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorBindingFlags bindingFlag =
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{};
            flagsCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            flagsCI.bindingCount = 1;
            flagsCI.pBindingFlags = &bindingFlag;

            VkDescriptorSetLayoutCreateInfo set1LayoutCI{};
            set1LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            set1LayoutCI.pNext = &flagsCI;
            set1LayoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            set1LayoutCI.bindingCount = 1;
            set1LayoutCI.pBindings = &b1;
            vkCreateDescriptorSetLayout(m_Device, &set1LayoutCI, nullptr, &m_GlobalSet1Layout);
        }

        // Allocate the bindless set 1 from the update-after-bind pool
        {
            VulkanDescriptorSet bindlessSet = m_BindlessAllocator->Allocate(m_GlobalSet1Layout, 1);
            m_BindlessSet = bindlessSet.GetHandle();
        }

        // Create the global set 2 (ModelData dynamic UBO) shared across all pipelines.
        // All pipelines that declare set 2 point to the same UBO, so one shared descriptor
        // set per frame is sufficient; pipelines no longer allocate their own.
        {
            VkDescriptorSetLayoutBinding set2Binding{};
            set2Binding.binding = 0;
            set2Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            set2Binding.descriptorCount = 1;
            set2Binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            m_GlobalSet2Layout = m_LayoutCache->GetOrCreate({set2Binding});

            for (int i = 0; i < maxFramesInFlight; i++) {
                m_GlobalSet2[i] = m_DescriptorAllocator->Allocate(m_GlobalSet2Layout, 2);
                m_GlobalSet2[i].Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_ModelDataUBOs[i].buffer, 0,
                                      m_ModelUBOSlotSize);
                m_GlobalSet2[i].Flush(m_Device);
            }
        }

        // Create a 1x1 black RGBA fallback image for unbound set-1 descriptor slots
        {
            VkImageCreateInfo imageCI{};
            imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCI.imageType = VK_IMAGE_TYPE_2D;
            imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageCI.extent = {1, 1, 1};
            imageCI.mipLevels = 1;
            imageCI.arrayLayers = 1;
            imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(m_VmaAllocator, &imageCI, &allocCI, &m_DummyImage, &m_DummyAllocation, nullptr);

            // Transition to SHADER_READ_ONLY_OPTIMAL via a one-time command
            VkCommandBufferAllocateInfo cbAllocInfo{};
            cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cbAllocInfo.commandPool = m_CommandPool;
            cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cbAllocInfo.commandBufferCount = 1;
            VkCommandBuffer cb = VK_NULL_HANDLE;
            vkAllocateCommandBuffers(m_Device, &cbAllocInfo, &cb);
            VkCommandBufferBeginInfo cbBegin{};
            cbBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cbBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cb, &cbBegin);
            VkImageMemoryBarrier2 barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_DummyImage;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
            barrier.srcAccessMask = 0;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            VkDependencyInfo depInfo{};
            depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depInfo.imageMemoryBarrierCount = 1;
            depInfo.pImageMemoryBarriers = &barrier;
            vkCmdPipelineBarrier2(cb, &depInfo);
            vkEndCommandBuffer(cb);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cb;
            vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_GraphicsQueue);
            vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &cb);

            VkImageViewCreateInfo viewCI{};
            viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCI.image = m_DummyImage;
            viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewCI.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewCI.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            vkCreateImageView(m_Device, &viewCI, nullptr, &m_DummyImageView);

            VkSamplerCreateInfo samplerCI{};
            samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCI.magFilter = VK_FILTER_NEAREST;
            samplerCI.minFilter = VK_FILTER_NEAREST;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            vkCreateSampler(m_Device, &samplerCI, nullptr, &m_DummySampler);
        }

        // --- Create global pipeline layout covering all 3 sets (used by Begin to bind Set 0 and Set 1) ---
        {
            VkDescriptorSetLayout layouts[] = {m_GlobalSet0Layout, m_GlobalSet1Layout, m_GlobalSet2Layout};
            VkPushConstantRange pushConstRange{};
            pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstRange.offset = 0;
            pushConstRange.size = 112;
            VkPipelineLayoutCreateInfo layoutCI{};
            layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutCI.setLayoutCount = 3;
            layoutCI.pSetLayouts = layouts;
            layoutCI.pushConstantRangeCount = 1;
            layoutCI.pPushConstantRanges = &pushConstRange;
            if (vkCreatePipelineLayout(m_Device, &layoutCI, nullptr, &m_GlobalFrameLayout) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create global frame pipeline layout!");
        }

        // --- Allocate and initialize Set 0 for each frame ---
        for (int i = 0; i < maxFramesInFlight; i++) {
            m_GlobalSet0[i] = m_BindlessAllocator->Allocate(m_GlobalSet0Layout, 0);
            if (!m_GlobalSet0[i].IsValid())
                return RenderInitError(RenderInitStatus::Failed, "Failed to allocate global Set 0!");

            // b0: FrameData UBO
            m_GlobalSet0[i].Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_FrameDataUBOs[i].buffer, 0,
                                  sizeof(Dodo::FrameData));
            // b3: CsmData UBO
            m_GlobalSet0[i].Write(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_CsmDataUBOs[i].buffer, 0,
                                  sizeof(Dodo::CsmData));
            // b5: shadow map placeholder (dummy 2D view until shadow pass runs)
            m_GlobalSet0[i].Write(5, m_DummyImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            // b6: irradiance/env map placeholder (dummy view until skybox initializes)
            m_GlobalSet0[i].Write(6, m_DummyImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_GlobalSet0[i].Flush(m_Device);
        }

        // Write the 6 default samplers to Set 0 binding 4 for all frames.
        // RegisterSampler already called above; now write each into both frame sets.
        for (uint32_t idx = 0; idx < (uint32_t)m_RegisteredSamplers.size(); idx++) {
            VkDescriptorImageInfo info{m_RegisteredSamplers[idx], VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};
            for (int i = 0; i < maxFramesInFlight; i++) {
                VkWriteDescriptorSet w{};
                w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                w.dstSet = m_GlobalSet0[i].GetHandle();
                w.dstBinding = 4;
                w.dstArrayElement = idx;
                w.descriptorCount = 1;
                w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                w.pImageInfo = &info;
                vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
            }
        }

        // Write dummy 2D view to bindless Set 1 slot 0 (fallback for unregistered texture handles).
        {
            VkDescriptorImageInfo dummyInfo{VK_NULL_HANDLE, m_DummyImageView,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            VkWriteDescriptorSet w{};
            w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            w.dstSet = m_BindlessSet;
            w.dstBinding = 0;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            w.pImageInfo = &dummyInfo;
            vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::InitImGui()
    {
        m_Context.InitializeImGui();

        VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
        poolInfo.pPoolSizes = poolSizes;

        if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
            return RenderInitError(RenderInitStatus::Failed, "Failed to create ImGui descriptor pool!");

        VkPipelineRenderingCreateInfo ImGuiPipelineInfo = {};
        ImGuiPipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        ImGuiPipelineInfo.colorAttachmentCount = 1;
        ImGuiPipelineInfo.pColorAttachmentFormats = &m_SwapChainImageFormat;

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        ImGui_ImplVulkan_InitInfo vulkanInfo = {};
        vulkanInfo.Instance = m_VkInstance;
        vulkanInfo.PhysicalDevice = m_PhysicalDevice;
        vulkanInfo.Device = m_Device;
        vulkanInfo.QueueFamily = indices.graphicsFamily.value();
        vulkanInfo.Queue = m_GraphicsQueue;
        vulkanInfo.DescriptorPool = m_ImGuiDescriptorPool;
        vulkanInfo.MinImageCount = 2;
        vulkanInfo.ImageCount = static_cast<uint32_t>(m_SwapChainImages.size());
        vulkanInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        vulkanInfo.UseDynamicRendering = true;
        vulkanInfo.PipelineInfoMain.PipelineRenderingCreateInfo = ImGuiPipelineInfo;

        // (Volk) Load functions
        ImGui_ImplVulkan_LoadFunctions(
            0,
            [](const char* function_name, void* vulkan_instance) {
                return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
            },
            &m_VkInstance);

        ImGui_ImplVulkan_Init(&vulkanInfo);

        m_ImGuiActive = true;
        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VulkanRenderAPI::InitTimestampPools()
    {
        VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &props2);

        if (!props2.properties.limits.timestampComputeAndGraphics) {
            DD_INFO("GPU does not support timestamp queries; GPU timings will be unavailable.");
            return RenderInitError(RenderInitStatus::Success);
        }

        m_TimestampPeriodNs = props2.properties.limits.timestampPeriod;

        VkQueryPoolCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
        ci.queryCount = maxTimestampQueries;

        for (int i = 0; i < maxFramesInFlight; i++) {
            if (vkCreateQueryPool(m_Device, &ci, nullptr, &m_TimestampPools[i]) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create timestamp query pool!");
            vkResetQueryPool(m_Device, m_TimestampPools[i], 0, maxTimestampQueries);
        }

        m_TimestampsSupported = true;
        return RenderInitError(RenderInitStatus::Success);
    }

    void VulkanRenderAPI::ReadTimestamps()
    {
        if (!m_TimestampsSupported) return;

        uint64_t ts[maxTimestampQueries] = {};
        VkResult r = vkGetQueryPoolResults(m_Device, m_TimestampPools[m_CurrentFrame], 0, maxTimestampQueries,
                                           sizeof(ts), ts, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
        if (r != VK_SUCCESS) return;

        auto toMs = [&](uint32_t slot) -> float {
            uint64_t begin = ts[slot * 2];
            uint64_t end = ts[slot * 2 + 1];
            if (end < begin) return 0.0f;
            return static_cast<float>((end - begin) * m_TimestampPeriodNs / 1e6);
        };

        m_GpuTimings.frameMs = toMs(static_cast<uint32_t>(Dodo::GpuTimestampSlot::Frame));
        m_GpuTimings.shadowMs = toMs(static_cast<uint32_t>(Dodo::GpuTimestampSlot::Shadow));
        m_GpuTimings.sceneMs = toMs(static_cast<uint32_t>(Dodo::GpuTimestampSlot::Scene));
        m_GpuTimings.postEffectMs = toMs(static_cast<uint32_t>(Dodo::GpuTimestampSlot::PostEffect));
    }

    void VulkanRenderAPI::BeginTimestamp(Dodo::GpuTimestampSlot slot)
    {
        if (!m_TimestampsSupported) return;
        vkCmdWriteTimestamp2(m_Frames[m_CurrentFrame].commandBuffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                             m_TimestampPools[m_CurrentFrame], static_cast<uint32_t>(slot) * 2);
    }

    void VulkanRenderAPI::EndTimestamp(Dodo::GpuTimestampSlot slot)
    {
        if (!m_TimestampsSupported) return;
        vkCmdWriteTimestamp2(m_Frames[m_CurrentFrame].commandBuffer, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                             m_TimestampPools[m_CurrentFrame], static_cast<uint32_t>(slot) * 2 + 1);
    }

    void VulkanRenderAPI::Begin()
    {
        if (m_SwapChainNeedsRecreation) {
            // Note: this is probably not the best way to do this. We want to program to still run while the window is
            // minmized
            int width = 0, height = 0;
            m_Context.GetFrameBufferSize(&width, &height);
            while (width == 0 || height == 0) {
                m_Context.GetFrameBufferSize(&width, &height);
                m_Context.WaitEvents(); // Wait for application to be unminimized
            }

            RecreateSwapChain();
            m_SwapChainNeedsRecreation = false;
        }

        VkFence inFlightFence = m_Frames[m_CurrentFrame].inFlightFence;
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;

        vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_Device, 1, &inFlightFence);

        ReadTimestamps();

        // Reset per-frame state now that the GPU has finished with this frame slot
        m_BoundPipeline = VK_NULL_HANDLE;
        m_BoundPipelinePtr = nullptr;
        m_ModelUBOCursor = 0;
        m_LastModelOffset = 0;
        m_IsRendering = false;
        vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_Frames[m_CurrentFrame].imageAvailableSemaphore,
                              VK_NULL_HANDLE, &m_CurrentImageIndex);

        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
            DD_ERR("failed to begin recording command buffer!");
            return;
        }

        // Bind Set 0 (FrameData + CsmData + samplers + shadow + env) and Set 1 (bindless texture heap)
        // once for the entire frame. All pipeline layouts share the same global layouts for sets 0-2,
        // so these bindings are never invalidated by pipeline switches.
        m_GlobalSet0[m_CurrentFrame].Bind(cmd, m_GlobalFrameLayout);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GlobalFrameLayout,
                                1, 1, &m_BindlessSet, 0, nullptr);

        if (m_TimestampsSupported) vkCmdResetQueryPool(cmd, m_TimestampPools[m_CurrentFrame], 0, maxTimestampQueries);
    }

    void VulkanRenderAPI::End()
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        uint32_t imageIndex = m_CurrentImageIndex;

        vkCmdEndRendering(cmd);
        m_IsRendering = false;

        // TODO: Deprecated stuff, use VkImageMemoryBarrier2KHR instead
        // Transition swapchain image back to present layout
        VkImageMemoryBarrier2 presentBarrier{};
        presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.image = m_SwapChainImages[imageIndex];
        presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentBarrier.subresourceRange.baseMipLevel = 0;
        presentBarrier.subresourceRange.levelCount = 1;
        presentBarrier.subresourceRange.baseArrayLayer = 0;
        presentBarrier.subresourceRange.layerCount = 1;
        presentBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        presentBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        presentBarrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        presentBarrier.dstAccessMask = 0;
        VkDependencyInfo presentDepInfo{};
        presentDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        presentDepInfo.imageMemoryBarrierCount = 1;
        presentDepInfo.pImageMemoryBarriers = &presentBarrier;
        vkCmdPipelineBarrier2(cmd, &presentDepInfo);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_Frames[m_CurrentFrame].imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[imageIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_Frames[m_CurrentFrame].inFlightFence) != VK_SUCCESS) {
            DD_ERR("failed to submit draw command buffer!");
            return;
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_SwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_PresentQueue, &presentInfo);

        m_FrameEpoch++;
        m_CurrentFrame = (m_CurrentFrame + 1) % maxFramesInFlight;
    }

    void VulkanRenderAPI::ClearColor(float r, float g, float b) const {}
    void VulkanRenderAPI::Viewport(uint width, uint height) const {}
    void VulkanRenderAPI::BindCubeMap(uint slot, Ref<CubeMap> cubemap)
    {
        if (!cubemap) return;
        VkImageView view = cubemap->GetImageView();
        if (!view) return;
        // slot 7 = irradiance map (binding 7), all others = environment map (binding 6).
        uint32_t dstBinding = (slot == 7) ? 7 : 6;
        VkDescriptorImageInfo info{VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        for (int i = 0; i < maxFramesInFlight; i++) {
            VkWriteDescriptorSet w{};
            w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            w.dstSet = m_GlobalSet0[i].GetHandle();
            w.dstBinding = dstBinding;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            w.pImageInfo = &info;
            vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
        }
    }

    void VulkanRenderAPI::BindTexture(uint slot, Ref<Texture> texture)
    {
        if (slot >= (uint)TextureSlot::Count || !texture) return;
        m_PendingTextureHandles[slot] = texture->GetBindlessHandle();
    }

    void VulkanRenderAPI::BindTextureSampler(uint /*slot*/, Ref<TextureSampler> sampler)
    {
        if (!sampler) return;
        m_PendingSamplerHandle = sampler->GetSamplerIndex();
    }

    void VulkanRenderAPI::BindFrameBufferTexture(uint slot, Ref<FrameBuffer> framebuffer)
    {
        if (!framebuffer) return;
        auto* vkFB = static_cast<VulkanFrameBuffer*>(framebuffer.get());
        if (!vkFB->HasColor()) {
            // Depth-only framebuffer (shadow map): write to Set 0 binding 5.
            VkImageView depthView = vkFB->GetDepthImageView();
            if (!depthView) return;
            VkDescriptorImageInfo info{VK_NULL_HANDLE, depthView,
                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
            for (int i = 0; i < maxFramesInFlight; i++) {
                VkWriteDescriptorSet w{};
                w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                w.dstSet = m_GlobalSet0[i].GetHandle();
                w.dstBinding = 5;
                w.dstArrayElement = 0;
                w.descriptorCount = 1;
                w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                w.pImageInfo = &info;
                vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
            }
        } else {
            // Color framebuffer: register in bindless heap and store handle at the given slot.
            if (slot >= 8) return;
            uint32_t handle = RegisterImageView(vkFB->GetColorImageView());
            m_PendingTextureHandles[slot] = handle;
        }
    }

    void VulkanRenderAPI::BindPipeline(Ref<Pipeline> pipeline)
    {
        memset(m_PendingTextureHandles, 0, sizeof(m_PendingTextureHandles));
        m_PendingSamplerHandle = 0;
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (pipeline->m_Pipeline != m_BoundPipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_Pipeline);
            m_BoundPipeline = pipeline->m_Pipeline;
            m_BoundPipelineLayout = pipeline->m_Layout;
            m_BoundPipelinePtr = pipeline.get();
        }
    }

    void VulkanRenderAPI::PushConstants(const void* data, size_t size)
    {
        if (m_BoundPipelineLayout == VK_NULL_HANDLE) return;
        vkCmdPushConstants(m_Frames[m_CurrentFrame].commandBuffer, m_BoundPipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint32_t)size, data);
    }

    void VulkanRenderAPI::SetFrameData(const Dodo::FrameData& data)
    {
        if (m_FrameDataUBOs[m_CurrentFrame].mapped)
            memcpy(m_FrameDataUBOs[m_CurrentFrame].mapped, &data, sizeof(Dodo::FrameData));
    }

    void VulkanRenderAPI::SetCSMData(const Dodo::CsmData& data)
    {
        if (m_CsmDataUBOs[m_CurrentFrame].mapped)
            memcpy(m_CsmDataUBOs[m_CurrentFrame].mapped, &data, sizeof(Dodo::CsmData));
    }

    void VulkanRenderAPI::SetDrawData(const DrawData& data)
    {
        if (!m_ModelDataUBOs[m_CurrentFrame].mapped) return;

        // Expand Mat4 model and Mat3 normal into GPU-aligned GPUModelData (160 bytes).
        GPUModelData gpu{};
        memcpy(gpu.model, data.model.m_Elements, sizeof(gpu.model));
        // Mat3 is column-major (m_Columns[3] of Vec3); pad each column into a vec4 row.
        for (int c = 0; c < 3; c++) {
            gpu.normal[c * 4 + 0] = data.normalMatrix.m_Columns[c].x;
            gpu.normal[c * 4 + 1] = data.normalMatrix.m_Columns[c].y;
            gpu.normal[c * 4 + 2] = data.normalMatrix.m_Columns[c].z;
        }

        gpu.albedoIdx   = m_PendingTextureHandles[(uint)TextureSlot::Albedo];
        gpu.roughIdx    = m_PendingTextureHandles[(uint)TextureSlot::Roughness];
        gpu.normalIdx   = m_PendingTextureHandles[(uint)TextureSlot::Normal];
        gpu.metallicIdx = m_PendingTextureHandles[(uint)TextureSlot::Metallic];
        gpu.aoIdx       = m_PendingTextureHandles[(uint)TextureSlot::Ao];
        gpu.specIdx     = m_PendingTextureHandles[(uint)TextureSlot::Spec];
        gpu.samplerIdx  = m_PendingSamplerHandle;

        uint32_t slot = m_ModelUBOCursor < maxDrawsPerFrame ? m_ModelUBOCursor++ : maxDrawsPerFrame - 1;
        m_LastModelOffset = slot * m_ModelUBOSlotSize;

        auto* dst = static_cast<uint8_t*>(m_ModelDataUBOs[m_CurrentFrame].mapped) + m_LastModelOffset;
        memcpy(dst, &gpu, sizeof(gpu));
    }

    void VulkanRenderAPI::BindVertexBuffer(const Ref<VertexBuffer>& vb)
    {
        VkDeviceSize offset = 0;
        VkBuffer buf = vb->GetBuffer();
        vkCmdBindVertexBuffers(m_Frames[m_CurrentFrame].commandBuffer, 0, 1, &buf, &offset);
    }

    void VulkanRenderAPI::BindIndexBuffer(const Ref<IndexBuffer>& ib)
    {
        vkCmdBindIndexBuffer(m_Frames[m_CurrentFrame].commandBuffer, ib->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanRenderAPI::DrawIndices(uint count)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (m_BoundPipelinePtr)
            m_BoundPipelinePtr->BindObjectSet(cmd, m_GlobalSet2[m_CurrentFrame], m_LastModelOffset);
        vkCmdDrawIndexed(cmd, count, 1, 0, 0, 0);
    }

    void VulkanRenderAPI::DrawIndicesInstanced(uint count, uint instanceCount)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (m_BoundPipelinePtr)
            m_BoundPipelinePtr->BindObjectSet(cmd, m_GlobalSet2[m_CurrentFrame], m_LastModelOffset);
        vkCmdDrawIndexed(cmd, count, instanceCount, 0, 0, 0);
    }

    void VulkanRenderAPI::DrawArray(uint count)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (m_BoundPipelinePtr)
            m_BoundPipelinePtr->BindObjectSet(cmd, m_GlobalSet2[m_CurrentFrame], m_LastModelOffset);
        vkCmdDraw(cmd, count, 1, 0, 0);
    }

    void VulkanRenderAPI::DefaultFrameBuffer()
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        vkCmdEndRendering(cmd);

        if (m_BoundFrameBuffer) {
            m_BoundFrameBuffer->TransitionToReadable(cmd);
            m_BoundFrameBuffer = nullptr;
        }

        // Transition swapchain image to color attachment layout before rendering to it
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_SwapChainImages[m_CurrentImageIndex];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = 0;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        VkDependencyInfo swapDepInfo{};
        swapDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        swapDepInfo.imageMemoryBarrierCount = 1;
        swapDepInfo.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(cmd, &swapDepInfo);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = m_SwapChainImageViews[m_CurrentImageIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0, 0}, m_SwapChainExtent};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(cmd, &renderingInfo);
        m_IsRendering = true;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChainExtent.width);
        viewport.height = static_cast<float>(m_SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_SwapChainExtent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderAPI::BindFrameBuffer(Ref<FrameBuffer> framebuffer)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (m_IsRendering) vkCmdEndRendering(cmd);

        auto* vkFB = static_cast<VulkanFrameBuffer*>(framebuffer.get());

        if (m_BoundFrameBuffer && m_BoundFrameBuffer != vkFB) m_BoundFrameBuffer->TransitionToReadable(cmd);

        vkFB->TransitionToRenderTarget(cmd);
        m_BoundFrameBuffer = vkFB;

        VkExtent2D extent = vkFB->GetExtent();

        VkRenderingAttachmentInfo colorAttachment{};
        VkRenderingAttachmentInfo depthAttachment{};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0, 0}, extent};
        renderingInfo.layerCount = vkFB->GetLayerCount();

        if (vkFB->HasColor()) {
            colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachment.imageView = vkFB->GetColorImageView();
            colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &colorAttachment;
        }

        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = vkFB->GetDepthImageView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = {1.0f, 0};
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(cmd, &renderingInfo);
        m_IsRendering = true;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderAPI::SetViewport(uint width, uint height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        m_SwapChainNeedsRecreation = true;
    }
    void VulkanRenderAPI::SetViewport(uint width, uint height, uint posX, uint posY)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        m_ViewportPosX = posX;
        m_ViewportPosY = posY;
        m_SwapChainNeedsRecreation = true;
    }

    Ref<Pipeline> VulkanRenderAPI::CreatePipeline(const PipelineDesc& desc, AssetManager& assets)
    {
        const ShaderAsset& shader = assets.GetShaderAsset(desc.shaderID);
        // Depth-only pipelines have no color attachment; other pipelines default to the HDR offscreen
        // buffer format (R16G16B16A16_SFLOAT) unless they explicitly target the swapchain.
        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
        if (!desc.depthOnly)
            if (desc.renderToSwapchain)
                colorFormat = m_SwapChainImageFormat;
            else if (desc.colorFormat == FrameBufferColorFormat::RGBA8)
                colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
            else
                colorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        return std::make_shared<VulkanPipeline>(m_Device, colorFormat, VK_FORMAT_D32_SFLOAT, shader, desc,
                                                m_GlobalSet0Layout, m_GlobalSet1Layout, m_GlobalSet2Layout,
                                                *m_LayoutCache, *m_DescriptorAllocator);
    }

    Ref<VertexBuffer> VulkanRenderAPI::CreateVertexBuffer(const float* vertices, uint size,
                                                          const BufferProperties& prop)
    {
        return std::make_shared<VulkanVertexBuffer>(vertices, size, prop, m_Device, m_VmaAllocator, m_CommandPool,
                                                    m_GraphicsQueue);
    }

    Ref<IndexBuffer> VulkanRenderAPI::CreateIndexBuffer(const uint* indices, uint count)
    {
        return std::make_shared<VulkanIndexBuffer>(indices, count, m_Device, m_VmaAllocator, m_CommandPool,
                                                   m_GraphicsQueue);
    }

    void VulkanRenderAPI::SubmitTextureBatch()
    {
        if (!m_UploadBatchActive) return;

        vkEndCommandBuffer(m_UploadCmdBuf);
        m_UploadBatchActive = false;

        if (m_UploadBatchTextures.empty() && m_UploadBatchCubeMaps.empty())
            return;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_UploadCmdBuf;
        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_UploadFence);

        m_UploadFencePending = true;
    }

    uint32_t VulkanRenderAPI::RegisterImageView(VkImageView view)
    {
        if (!view) return 0;
        auto it = m_BindlessHandleMap.find(view);
        if (it != m_BindlessHandleMap.end()) return it->second;

        uint32_t slot;
        if (!m_BindlessFreeList.empty()) {
            slot = m_BindlessFreeList.back();
            m_BindlessFreeList.pop_back();
        } else if (m_BindlessNextSlot < k_BindlessMaxTextures) {
            slot = m_BindlessNextSlot++;
        } else {
            DD_WARN("Bindless texture heap exhausted (max {})", k_BindlessMaxTextures);
            return 0;
        }

        m_BindlessHandleMap[view] = slot;
        VkDescriptorImageInfo info{VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = m_BindlessSet;
        w.dstBinding = 0;
        w.dstArrayElement = slot;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        w.pImageInfo = &info;
        vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
        return slot;
    }

    void VulkanRenderAPI::RegisterTexture(VulkanTexture& texture)
    {
        if (texture.m_BindlessHandle != 0) return;
        texture.m_BindlessHandle = RegisterImageView(texture.GetImageView());
    }

    void VulkanRenderAPI::RegisterCubeMap(VulkanCubeMap& cubeMap)
    {
        // Cubemaps go to Set 0 bindings 6/7 via BindCubeMap, not the bindless array.
        // FinalizeUpload is already called by PollTextureBatch before this; nothing extra needed.
        (void)cubeMap;
    }

    uint32_t VulkanRenderAPI::RegisterSampler(VkSampler sampler)
    {
        auto it = m_SamplerIndexMap.find(sampler);
        if (it != m_SamplerIndexMap.end()) return it->second;

        if (m_ActiveSamplerCount >= k_BindlessMaxSamplers) {
            DD_ERR("Sampler table full ({} slots): returning slot 0 as fallback", k_BindlessMaxSamplers);
            return 0;
        }

        uint32_t idx = m_ActiveSamplerCount++;
        m_RegisteredSamplers.push_back(sampler);
        m_SamplerIndexMap[sampler] = idx;

        // Write the new sampler into Set 0 binding 4 for both frame sets.
        // If Set 0 hasn't been allocated yet (during InitDescriptors), the bulk write will follow.
        if (m_GlobalSet0[0].IsValid()) {
            VkDescriptorImageInfo info{sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};
            for (int i = 0; i < maxFramesInFlight; i++) {
                VkWriteDescriptorSet w{};
                w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                w.dstSet = m_GlobalSet0[i].GetHandle();
                w.dstBinding = 4;
                w.dstArrayElement = idx;
                w.descriptorCount = 1;
                w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                w.pImageInfo = &info;
                vkUpdateDescriptorSets(m_Device, 1, &w, 0, nullptr);
            }
        }
        return idx;
    }

    void VulkanRenderAPI::BeginUploadBatch()
    {
        if (m_UploadFencePending)
        {
            vkWaitForFences(m_Device, 1, &m_UploadFence, VK_TRUE, UINT64_MAX);
            vkResetFences(m_Device, 1, &m_UploadFence);
            m_UploadFencePending = false;
            for (auto& tex : m_UploadBatchTextures)
            {
                tex->FinalizeUpload();
                RegisterTexture(*static_cast<VulkanTexture*>(tex.get()));
            }
            m_UploadBatchTextures.clear();
            for (auto& cm : m_UploadBatchCubeMaps)
            {
                cm->FinalizeUpload();
                RegisterCubeMap(*static_cast<VulkanCubeMap*>(cm.get()));
            }
            m_UploadBatchCubeMaps.clear();
        }
        vkResetCommandBuffer(m_UploadCmdBuf, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(m_UploadCmdBuf, &beginInfo);
        m_UploadBatchActive = true;
    }

    bool VulkanRenderAPI::PollTextureBatch()
    {
        if (!m_UploadFencePending) return true;

        VkResult result = vkGetFenceStatus(m_Device, m_UploadFence);
        if (result != VK_SUCCESS) return false;

        vkResetFences(m_Device, 1, &m_UploadFence);
        m_UploadFencePending = false;

        for (auto& tex : m_UploadBatchTextures) {
            tex->FinalizeUpload();
            RegisterTexture(*static_cast<VulkanTexture*>(tex.get()));
        }
        m_UploadBatchTextures.clear();

        for (auto& cm : m_UploadBatchCubeMaps) {
            cm->FinalizeUpload();
            RegisterCubeMap(*static_cast<VulkanCubeMap*>(cm.get()));
        }
        m_UploadBatchCubeMaps.clear();

        return true;
    }

    Ref<Texture> VulkanRenderAPI::CreateTexture(const uchar* data, const TextureProperties& prop)
    {
        if (!m_UploadBatchActive)
            BeginUploadBatch();
        auto tex = std::make_shared<VulkanTexture>(data, prop, m_Device, m_VmaAllocator, m_UploadCmdBuf);
        m_UploadBatchTextures.push_back(tex);
        return tex;
    }

    static uint32_t MakeSamplerCacheKey(const SamplerProperties& p)
    {
        uint32_t h = (static_cast<uint32_t>(p.m_Filter) << 8)
                   | (static_cast<uint32_t>(p.m_WrapU)  << 4)
                   | (static_cast<uint32_t>(p.m_WrapV));
        for (int i = 0; i < 4; i++) {
            uint32_t bits;
            std::memcpy(&bits, &p.m_BorderColor[i], sizeof(bits));
            h ^= bits;
        }
        return h;
    }

    Ref<TextureSampler> VulkanRenderAPI::CreateSampler(const SamplerProperties& prop)
    {
        uint32_t key = MakeSamplerCacheKey(prop);
        auto it = m_SamplerCache.find(key);
        if (it != m_SamplerCache.end()) return it->second;

        auto sampler = std::make_shared<VulkanSampler>(prop, m_Device);
        sampler->m_SamplerIndex = RegisterSampler(sampler->m_Sampler);
        m_SamplerCache.emplace(key, sampler);
        return sampler;
    }

    Ref<CubeMap> VulkanRenderAPI::CreateCubeMap(const CubeMapData& data)
    {
        if (!m_UploadBatchActive)
            BeginUploadBatch();
        auto cm = std::make_shared<VulkanCubeMap>(data, m_Device, m_VmaAllocator, m_UploadCmdBuf);
        m_UploadBatchCubeMaps.push_back(cm);
        return cm;
    }

    Ref<CubeMap> VulkanRenderAPI::CreateCubeMapFromEquirectangular(Ref<Texture> equirect, uint faceSize,
                                                                   AssetManager& assets)
    {
        ShaderID shaderID =
            assets.LoadShaderFromPath("res/shader/builtin/Passes/EquirectangularTransform.slang");
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = shaderID;
        pipelineDesc.culling  = CullMode::None;
        pipelineDesc.useBindlessHeap = false; // GPU pass: manages its own Set 1 from shader reflection
        auto pipeline =
            std::static_pointer_cast<VulkanPipeline>(assets.GetPipeline(assets.CreatePipeline(pipelineDesc, *this)));
        VkDescriptorSetLayout set1Layout = pipeline->m_SetLayouts[1];

        static const float s_CubeVertices[] = {
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
        };
        auto vbo = std::static_pointer_cast<VulkanVertexBuffer>(
            CreateVertexBuffer(s_CubeVertices, sizeof(s_CubeVertices), BufferProperties({{"POSITION", 3}})));
        auto sampler = std::static_pointer_cast<VulkanSampler>(CreateSampler(SamplerProperties(
            SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
            SamplerWrapMode::WRAP_CLAMP_TO_EDGE)));

        SubmitTextureBatch();
        auto pass = std::make_unique<VulkanEquirectangularPass>(
            equirect, faceSize, pipeline, set1Layout, vbo, sampler, MakeGpuPassContext());
        Ref<CubeMap> result = pass->GetResult();
        m_GpuPassQueue->Submit(std::move(pass));
        return result;
    }

    VulkanGpuPassContext VulkanRenderAPI::MakeGpuPassContext()
    {
        return VulkanGpuPassContext{
            m_Device,
            m_VmaAllocator,
            m_CommandPool,
            m_GraphicsQueue,
            m_DescriptorAllocator.get(),
            m_BindlessAllocator.get(),
            m_GlobalSet0Layout,
            m_GlobalSet2Layout,
        };
    }

    void VulkanRenderAPI::SubmitGpuPass(std::unique_ptr<VulkanGpuPass> pass)
    {
        m_GpuPassQueue->Submit(std::move(pass));
    }

    void VulkanRenderAPI::PollGpuPasses()
    {
        m_GpuPassQueue->Poll();
    }

    void VulkanRenderAPI::WaitGpuPasses()
    {
        m_GpuPassQueue->WaitAll();
    }

    Ref<CubeMap> VulkanRenderAPI::CreateIrradianceMap(Ref<CubeMap> envMap, uint faceSize,
                                                      AssetManager& assets)
    {
        // Finalize any pending passes (e.g., the equirectangular pass) so the env cubemap
        // image is in SHADER_READ_ONLY_OPTIMAL before the convolution shader samples it.
        m_GpuPassQueue->WaitAll();

        ShaderID shaderID =
            assets.LoadShaderFromPath("res/shader/builtin/Passes/CubemapConvolution.slang");
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = shaderID;
        pipelineDesc.culling  = CullMode::None;
        pipelineDesc.useBindlessHeap = false; // GPU pass: manages its own Set 1 from shader reflection
        auto pipeline =
            std::static_pointer_cast<VulkanPipeline>(assets.GetPipeline(assets.CreatePipeline(pipelineDesc, *this)));
        VkDescriptorSetLayout set1Layout = pipeline->m_SetLayouts[1];

        static const float s_CubeVertices[] = {
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
        };
        auto vbo = std::static_pointer_cast<VulkanVertexBuffer>(
            CreateVertexBuffer(s_CubeVertices, sizeof(s_CubeVertices), BufferProperties({{"POSITION", 3}})));
        auto sampler = std::static_pointer_cast<VulkanSampler>(CreateSampler(SamplerProperties(
            SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
            SamplerWrapMode::WRAP_CLAMP_TO_EDGE)));

        auto vkEnvMap = std::static_pointer_cast<VulkanCubeMap>(envMap);
        auto pass = std::make_unique<VulkanCubemapConvolutionPass>(
            vkEnvMap, faceSize, pipeline, set1Layout, vbo, sampler, MakeGpuPassContext());
        Ref<CubeMap> result = pass->GetResult();
        m_GpuPassQueue->Submit(std::move(pass));
        return result;
    }

    Ref<FrameBuffer> VulkanRenderAPI::CreateFrameBuffer(const FrameBufferProperties& props)
    {
        return std::make_shared<VulkanFrameBuffer>(props, m_Device, m_VmaAllocator);
    }

    void* VulkanRenderAPI::GetFrameBufferImGuiTextureID(Ref<FrameBuffer> framebuffer)
    {
        auto* fb = static_cast<VulkanFrameBuffer*>(framebuffer.get());
        VkExtent2D current = fb->GetExtent();
        auto it = m_ImGuiFrameBufferEntries.find(fb);
        if (it != m_ImGuiFrameBufferEntries.end()) {
            if (it->second.extent.width == current.width && it->second.extent.height == current.height)
                return (void*)it->second.set;
            ImGui_ImplVulkan_RemoveTexture(it->second.set);
            m_ImGuiFrameBufferEntries.erase(it);
        }
        VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(
            fb->GetSampler(), fb->GetColorImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_ImGuiFrameBufferEntries[fb] = {set, current};
        return (void*)set;
    }

    void* VulkanRenderAPI::GetTextureImGuiID(Ref<Texture> texture)
    {
        auto* tex = static_cast<VulkanTexture*>(texture.get());
        auto it = m_ImGuiTextureEntries.find(tex);
        if (it != m_ImGuiTextureEntries.end())
            return (void*)it->second;
        VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(
            m_DummySampler, tex->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_ImGuiTextureEntries[tex] = set;
        return (void*)set;
    }

    void VulkanRenderAPI::ImGuiNewFrame() const
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VulkanRenderAPI::ImGuiEndFrame()
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;

        // Render ImGui into the currently active swapchain pass (started by DefaultFrameBuffer or Begin)
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        // Pass is ended by End()
    }

    /**
     * Checks if a physical device meets our requirements of features, queue families, and extensions
     */
    bool VulkanRenderAPI::IsDeviceSuitable(PhyisicalDeviceInfo device)
    {
        if (device.device == VK_NULL_HANDLE) return false;
        if (!device.features.geometryShader)
            return false; // Note: This might fail on MacOS even though the device supports geometry shaders, due to
                          // MoltenVK not reporting it correctly.
        if (!device.features.tessellationShader) return false;
        if (!device.features.depthClamp) return false;

        if (!device.indices.IsComplete()) return false;

        // Check if all required device extensions are supported
        std::vector<const char*> requiredExtensions = GetRequiredDeviceExtensions(device.device);
        if (!CheckDeviceExtensionSupport(device.device, requiredExtensions)) return false;

        // Check if swap chain is enough for presentation
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device.device);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) return false;

        return true;
    }

    /**
     * Takes two candidate physical devices and provides an ordering based on their type
     */
    bool VulkanRenderAPI::IsDeviceBetter(PhyisicalDeviceInfo bestDevice, PhyisicalDeviceInfo device)
    {
        if (bestDevice.device == VK_NULL_HANDLE) return true;

        if (bestDevice.properties.deviceType < device.properties.deviceType) {
            /**
             * VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
             * VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
             * VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
             * VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
             * VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
             *
             * Prefer higher valued devices with the exception of CPU devices
             */
            return device.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU;
        }

        // TODO: Selection based on VRAM:
        // https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
        return false;
    }

    /**
     * Finds the queue families supported by a physical device
     */
    QueueFamilyIndices VulkanRenderAPI::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount,
                                                            {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            bool graphics = queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &present);
            if (graphics && present) {
                indices.graphicsFamily = i;
                indices.presentFamily = i;
                break; // Note: not modular for more queues rn, but we prefer same family for graphics and present
            }
            if (graphics) indices.graphicsFamily = i;
            if (present) indices.presentFamily = i;

            i++;
        }

        return indices;
    }

    /**
     * Retrieves the required instance extensions, including those required by the window backend
     */
    std::vector<const char*> VulkanRenderAPI::GetRequiredExtensions()
    {
        std::vector<const char*> extensions = m_Context.GetExtensions();

        if (m_EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

// MoltenVK on MacOS requires this extension. The extension is not expected for renderdoc since it does not support
// MacOS, therefore it most be disabled in that case
#ifdef DD_MACOS
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        return extensions;
    }

    bool VulkanRenderAPI::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Build a hash set of available layer names
        std::unordered_set<std::string> availableLayerNames;
        availableLayerNames.reserve(layerCount);
        for (const auto& layer : availableLayers) {
            availableLayerNames.insert(layer.layerName);
        }

        // Store every missing extension for error reporting
        std::vector<const char*> missingLayers;
        for (const char* layerName : m_ValidationLayers) {
            if (availableLayerNames.find(layerName) == availableLayerNames.end()) {
                missingLayers.push_back(layerName);
            }
        }

        // If there are any missing extensions, log all of them and return false
        if (!missingLayers.empty()) {
            DD_ERR("Missing required layers:");
            for (const char* required : missingLayers) {
                DD_ERR("{}", required);
            }
            return false;
        }

        return true;
    }

    /**
     * Get required device extensions given the phyisical device.
     * Some extensions exist within the core after a certain version
     * so a version probe is necessary
     */
    std::vector<const char*> VulkanRenderAPI::GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(physicalDevice, &props2);

        if (VK_API_VERSION_MINOR(props2.properties.apiVersion) < 3) {
            // Dynamic rendering is built in core in 1.3+, otherwise we need the extension
            extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }

        extensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);

        return extensions;
    }

    bool VulkanRenderAPI::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                                      const std::vector<const char*>& requiredExtensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        //DD_INFO("Vulkan Device Extensions:");
        // Build a hash set of available extension names
        std::unordered_set<std::string> availableExtensionNames;
        availableExtensionNames.reserve(extensionCount);
        for (const auto& extension : availableExtensions) {
            availableExtensionNames.insert(extension.extensionName);
            //DD_INFO("{}", extension.extensionName);
        }

        // Store every missing extension for error reporting
        std::vector<const char*> missingExtensions;
        for (const char* required : requiredExtensions) {
            if (availableExtensionNames.find(required) == availableExtensionNames.end()) {
                missingExtensions.push_back(required);
            }
        }

        // If there are any missing extensions, log all of them and return false
        if (!missingExtensions.empty()) {
            DD_ERR("Physical device is missing required extensions:");
            for (const char* required : missingExtensions) {
                DD_ERR("{}", required);
            }
            return false;
        }

        return true;
    }

    SwapChainSupportDetails VulkanRenderAPI::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    bool VulkanRenderAPI::IsBetterSurfaceFormat(VkSurfaceFormatKHR candidate, VkSurfaceFormatKHR current) const
    {
        auto Score = [](VkSurfaceFormatKHR fmt) -> int {
            int score = 0;
            if (fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) score += 10;
            switch (fmt.format) {
                case VK_FORMAT_B8G8R8A8_UNORM: score += 4; break;
                case VK_FORMAT_R8G8B8A8_UNORM: score += 3; break;
                case VK_FORMAT_B8G8R8A8_SRGB:  score += 2; break;
                case VK_FORMAT_R8G8B8A8_SRGB:  score += 1; break;
                default: break;
            }
            return score;
        };
        return Score(candidate) > Score(current);
    }

    /**
     * Select the best surface format given available pixel formats
     */
    VkSurfaceFormatKHR VulkanRenderAPI::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        VkSurfaceFormatKHR best = availableFormats[0];
        for (size_t i = 1; i < availableFormats.size(); i++)
            if (IsBetterSurfaceFormat(availableFormats[i], best))
                best = availableFormats[i];
        return best;
    }

    /**
     * Select the best swap present mode given available present modes
     */
    VkPresentModeKHR VulkanRenderAPI::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &props2);

        if (props2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            for (const auto& mode : availablePresentModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
            }
        } else {
            for (const auto& mode : availablePresentModes) {
                if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanRenderAPI::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
            return capabilities.currentExtent;
        }

        int width, height;
        m_Context.GetFrameBufferSize(&width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void VulkanRenderAPI::RecreateSwapChain()
    {
        vkDeviceWaitIdle(m_Device);

        // Destroy old image views
        for (auto imageView : m_SwapChainImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);
        m_SwapChainImageViews.clear();

        // We destroy render semaphores here since they are sized to the swap chain image count and it is possible we
        // want to change between triple and double buffering
        for (auto& sem : m_RenderFinishedSemaphores)
            vkDestroySemaphore(m_Device, sem, nullptr);
        m_RenderFinishedSemaphores.clear();

        // Recreate swap chain with old swapchain so that data can be transferred if necessary
        VkSwapchainKHR oldSwapchain = m_SwapChain;
        CreateSwapChain(oldSwapchain);
        vkDestroySwapchainKHR(m_Device, oldSwapchain, nullptr);

        CreateImageViews();

        // Recreate render finished semaphores sized to new swapchain image count
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        m_RenderFinishedSemaphores.resize(m_SwapChainImages.size());
        for (auto& sem : m_RenderFinishedSemaphores)
            vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &sem);
    }

    /**
     * This function populates a VkDebugUtilsMessengerCreateInfoEXT structure with the desired settings for the debug
     * messenger. This function is used while setting up the debug messenger and when creating the Vulkan instance. This
     * ensures that the debug messenger is set up even during instance creation.
     */
    void VulkanRenderAPI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    VkResult VulkanRenderAPI::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                           const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void VulkanRenderAPI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                        const VkAllocationCallbacks* pAllocator)
    {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    /**
     * A callback function that is called by the Vulkan validation layers
     */
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderAPI::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            // DD_INFO("validation layer (verbose): {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            // DD_INFO("validation layer (info): {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            DD_WARN("validation layer (warning): {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            DD_ERR("validation layer (error): {}", pCallbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }
} // namespace Dodo::Platform