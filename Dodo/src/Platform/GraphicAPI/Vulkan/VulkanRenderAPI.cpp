#include "VulkanRenderAPI.h"
#include "pch.h"

#include "Core/Data/AssetManager.h"
#include "VulkanBuffer.h"
#include "VulkanCubeMap.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanSampler.h"
#include "VulkanTexture.h"

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
            if (m_TransientPools[i]) vkDestroyDescriptorPool(m_Device, m_TransientPools[i], nullptr);
        }
        if (m_AppDescriptorPool) vkDestroyDescriptorPool(m_Device, m_AppDescriptorPool, nullptr);
        if (m_DummySampler) vkDestroySampler(m_Device, m_DummySampler, nullptr);
        if (m_DummyImageView) vkDestroyImageView(m_Device, m_DummyImageView, nullptr);
        if (m_DummyImage) vmaDestroyImage(m_VmaAllocator, m_DummyImage, m_DummyAllocation);

        if (m_ImGuiActive) {
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
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dodo Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
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

        PhyisicalDeviceInfo bestDevice;
        bestDevice.device = VK_NULL_HANDLE;
        for (const auto& device : devices) {
            PhyisicalDeviceInfo deviceInfo;
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            deviceInfo.device = device;
            deviceInfo.properties = deviceProperties;
            deviceInfo.features = deviceFeatures;
            deviceInfo.indices = FindQueueFamilies(device);
            if (!IsDeviceSuitable(deviceInfo)) {
                DD_INFO("Device {} is not suitable", deviceProperties.deviceName);
                continue;
            }
            if (IsDeviceBetter(bestDevice, deviceInfo)) {
                m_PhysicalDevice = device;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            return RenderInitError(RenderInitStatus::Failed, "failed to find a suitable GPU!");
        }

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

        // Dynamic rendering feature
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
        dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeature.dynamicRendering = VK_TRUE;

        // We will specify device features here later
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &dynamicRenderingFeature;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (m_EnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

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
        VkPhysicalDeviceProperties devProps{};
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &devProps);
        uint32_t alignment = (uint32_t)devProps.limits.minUniformBufferOffsetAlignment;
        uint32_t rawSize = (uint32_t)sizeof(GPUModelData);
        m_ModelUBOSlotSize = (rawSize + alignment - 1) & ~(alignment - 1);

        // --- Application descriptor pool ---
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64},          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 64},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256},          {VK_DESCRIPTOR_TYPE_SAMPLER, 256},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256},
        };
        VkDescriptorPoolCreateInfo poolCI{};
        poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCI.maxSets = 256;
        poolCI.poolSizeCount = (uint32_t)std::size(poolSizes);
        poolCI.pPoolSizes = poolSizes;
        if (vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_AppDescriptorPool) != VK_SUCCESS)
            return RenderInitError(RenderInitStatus::Failed, "Failed to create application descriptor pool!");

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
        }

        // --- Per-frame transient pools for set-1 (material textures) ---
        VkDescriptorPoolSize transientSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxDrawsPerFrame * maxTextureSlots},
            {VK_DESCRIPTOR_TYPE_SAMPLER, maxDrawsPerFrame * maxTextureSlots},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxDrawsPerFrame * maxTextureSlots},
        };
        VkDescriptorPoolCreateInfo transientCI{};
        transientCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        transientCI.maxSets = maxDrawsPerFrame;
        transientCI.poolSizeCount = (uint32_t)std::size(transientSizes);
        transientCI.pPoolSizes = transientSizes;
        for (int i = 0; i < maxFramesInFlight; i++) {
            if (vkCreateDescriptorPool(m_Device, &transientCI, nullptr, &m_TransientPools[i]) != VK_SUCCESS)
                return RenderInitError(RenderInitStatus::Failed, "Failed to create transient descriptor pool!");
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
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_DummyImage;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &barrier);
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

        // Reset per-frame state now that the GPU has finished with this frame slot
        m_BoundPipeline = VK_NULL_HANDLE;
        m_BoundPipelinePtr = nullptr;
        m_ModelUBOCursor = 0;
        m_LastModelOffset = 0;
        m_TexturesDirty = false;
        if (m_TransientPools[m_CurrentFrame] != VK_NULL_HANDLE)
            vkResetDescriptorPool(m_Device, m_TransientPools[m_CurrentFrame], 0);

        vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_Frames[m_CurrentFrame].imageAvailableSemaphore,
                              VK_NULL_HANDLE, &m_CurrentImageIndex);

        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
            DD_ERR("failed to begin recording command buffer!");
            return;
        }

        // Transition swapchain image to color attachment layout
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = m_SwapChainImageViews[m_CurrentImageIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0, 0}, m_SwapChainExtent};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(cmd, &renderingInfo);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChainExtent.width);
        viewport.height = static_cast<float>(m_SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = m_SwapChainExtent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderAPI::End()
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        uint32_t imageIndex = m_CurrentImageIndex;

        vkCmdEndRendering(cmd);

        // TODO: Deprecated stuff, use VkImageMemoryBarrier2KHR instead
        // Transition swapchain image back to present layout
        VkImageMemoryBarrier presentBarrier = {};
        presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
        presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        presentBarrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &presentBarrier);

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

        m_CurrentFrame = (m_CurrentFrame + 1) % maxFramesInFlight;
    }

    void VulkanRenderAPI::ClearColor(float r, float g, float b) const {}
    void VulkanRenderAPI::Viewport(uint width, uint height) const {}
    void VulkanRenderAPI::BindCubeMap(uint slot, Ref<CubeMap> cubemap)
    {
        if (slot >= maxTextureSlots || !cubemap) return;
        m_PendingImageViews[slot] = cubemap->GetImageView();
        m_PendingIsCubeMap[slot] = true;
        m_PendingIsDepth[slot] = false;
        m_TexturesDirty = true;
    }

    void VulkanRenderAPI::BindTexture(uint slot, Ref<Texture> texture)
    {
        if (slot >= maxTextureSlots || !texture) return;
        m_PendingImageViews[slot] = texture->GetImageView();
        m_PendingIsCubeMap[slot] = false;
        m_PendingIsDepth[slot] = false;
        m_TexturesDirty = true;
    }

    void VulkanRenderAPI::BindTextureSampler(uint slot, Ref<TextureSampler> sampler)
    {
        if (slot >= maxTextureSlots || !sampler) return;
        m_PendingSamplers[slot] = sampler->GetSampler();
        m_TexturesDirty = true;
    }

    void VulkanRenderAPI::BindFrameBufferTexture(uint slot, Ref<FrameBuffer> framebuffer)
    {
        if (slot >= maxTextureSlots || !framebuffer) return;
        auto* vkFB = static_cast<VulkanFrameBuffer*>(framebuffer.get());
        const bool depthOnly = !vkFB->HasColor();
        m_PendingImageViews[slot] = depthOnly ? vkFB->GetDepthImageView() : vkFB->GetColorImageView();
        m_PendingSamplers[slot] = vkFB->GetSampler();
        m_PendingIsCubeMap[slot] = false;
        m_PendingIsDepth[slot] = depthOnly;
        m_TexturesDirty = true;
    }

    void VulkanRenderAPI::BindPipeline(Ref<Pipeline> pipeline)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (pipeline->m_Pipeline != m_BoundPipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_Pipeline);
            m_BoundPipeline = pipeline->m_Pipeline;
            m_BoundPipelineLayout = pipeline->m_Layout;
            m_BoundPipelinePtr = pipeline.get();
        }
        m_TexturesDirty = true;

        // Bind set-0 (FrameData + ModelData). Pipeline is a no-op if the shader does not declare set-0.
        m_BoundPipelinePtr->BindGlobalSet(cmd, m_CurrentFrame, m_LastModelOffset);
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

    void VulkanRenderAPI::SetDrawData(const DrawData& data)
    {
        if (!m_ModelDataUBOs[m_CurrentFrame].mapped) return;

        // Expand Mat4 model and Mat3 normal into GPU-aligned GPUModelData (128 bytes)
        GPUModelData gpu{};
        memcpy(gpu.model, data.model.m_Elements, sizeof(gpu.model));
        // Mat3 is column-major (m_Columns[3] of Vec3); pad each column into vec4
        for (int c = 0; c < 3; c++) {
            gpu.normal[c * 4 + 0] = data.normalMatrix.m_Columns[c].x;
            gpu.normal[c * 4 + 1] = data.normalMatrix.m_Columns[c].y;
            gpu.normal[c * 4 + 2] = data.normalMatrix.m_Columns[c].z;
            // gpu.normal[c * 4 + 3] stays 0
        }

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

        m_BoundPipelinePtr->BindMaterialSet(cmd, m_TransientPools[m_CurrentFrame], m_CurrentFrame,
                                            m_PendingImageViews, m_PendingSamplers,
                                            m_PendingIsCubeMap, m_PendingIsDepth, maxTextureSlots,
                                            m_DummyImageView, m_DummySampler);
        m_TexturesDirty = false;

        vkCmdDrawIndexed(cmd, count, 1, 0, 0, 0);
    }

    void VulkanRenderAPI::DrawArray(uint count)
    {
        VkCommandBuffer cmd = m_Frames[m_CurrentFrame].commandBuffer;
        if (m_BoundPipelinePtr) {
            m_BoundPipelinePtr->BindGlobalSet(cmd, m_CurrentFrame, m_LastModelOffset);
            m_BoundPipelinePtr->BindMaterialSet(cmd, m_TransientPools[m_CurrentFrame], m_CurrentFrame,
                                                m_PendingImageViews, m_PendingSamplers,
                                                m_PendingIsCubeMap, m_PendingIsDepth, maxTextureSlots,
                                                m_DummyImageView, m_DummySampler);
            m_TexturesDirty = false;
        }
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

        // Resume rendering to the swapchain image (already in COLOR_ATTACHMENT_OPTIMAL from Begin())
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
        vkCmdEndRendering(cmd);

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
        renderingInfo.layerCount = 1;

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
            colorFormat = desc.renderToSwapchain ? m_SwapChainImageFormat : VK_FORMAT_R16G16B16A16_SFLOAT;
        PipelineUBOHandles ubos{};
        for (int i = 0; i < maxFramesInFlight; i++) {
            ubos.frameDataBuffers[i] = m_FrameDataUBOs[i].buffer;
            ubos.modelDataBuffers[i] = m_ModelDataUBOs[i].buffer;
        }
        ubos.modelSlotSize = m_ModelUBOSlotSize;
        return std::make_shared<VulkanPipeline>(m_Device, colorFormat, VK_FORMAT_D32_SFLOAT, shader, desc, ubos);
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

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_UploadCmdBuf;
        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_UploadFence);

        m_UploadBatchActive = false;
        m_UploadFencePending = true;
    }

    bool VulkanRenderAPI::PollTextureBatch()
    {
        if (!m_UploadFencePending) return true;

        VkResult result = vkGetFenceStatus(m_Device, m_UploadFence);
        if (result == VK_SUCCESS) {
            vkResetFences(m_Device, 1, &m_UploadFence);
            m_UploadFencePending = false;

            for (auto& tex : m_UploadBatchTextures)
                tex->FinalizeUpload();
            m_UploadBatchTextures.clear();

            for (auto& cm : m_UploadBatchCubeMaps)
                cm->FinalizeUpload();
            m_UploadBatchCubeMaps.clear();

            return true;
        }
        return false;
    }

    Ref<Texture> VulkanRenderAPI::CreateTexture(uchar* data, const TextureProperties& prop)
    {
        if (!m_UploadBatchActive) {
            // Start a new batch lazily (handles fallback textures created outside the main texture loop)
            if (m_UploadFencePending) {
                // Previous batch not yet finalized: wait synchronously so we can reset the fence
                vkWaitForFences(m_Device, 1, &m_UploadFence, VK_TRUE, UINT64_MAX);
                vkResetFences(m_Device, 1, &m_UploadFence);
                m_UploadFencePending = false;
                for (auto& tex : m_UploadBatchTextures) tex->FinalizeUpload();
                m_UploadBatchTextures.clear();
                for (auto& cm : m_UploadBatchCubeMaps) cm->FinalizeUpload();
                m_UploadBatchCubeMaps.clear();
            }
            vkResetCommandBuffer(m_UploadCmdBuf, 0);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(m_UploadCmdBuf, &beginInfo);
            m_UploadBatchActive = true;
        }

        auto tex = std::make_shared<VulkanTexture>(data, prop, m_Device, m_VmaAllocator, m_UploadCmdBuf);
        m_UploadBatchTextures.push_back(tex);
        return tex;
    }

    Ref<TextureSampler> VulkanRenderAPI::CreateSampler(const SamplerProperties& prop)
    {
        return std::make_shared<VulkanSampler>(prop, m_Device);
    }

    Ref<CubeMap> VulkanRenderAPI::CreateCubeMap(const CubeMapData& data)
    {
        if (!m_UploadBatchActive) {
            if (m_UploadFencePending) {
                vkWaitForFences(m_Device, 1, &m_UploadFence, VK_TRUE, UINT64_MAX);
                vkResetFences(m_Device, 1, &m_UploadFence);
                m_UploadFencePending = false;
                for (auto& tex : m_UploadBatchTextures) tex->FinalizeUpload();
                m_UploadBatchTextures.clear();
                for (auto& cm : m_UploadBatchCubeMaps) cm->FinalizeUpload();
                m_UploadBatchCubeMaps.clear();
            }
            vkResetCommandBuffer(m_UploadCmdBuf, 0);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(m_UploadCmdBuf, &beginInfo);
            m_UploadBatchActive = true;
        }

        auto cm = std::make_shared<VulkanCubeMap>(data, m_Device, m_VmaAllocator, m_UploadCmdBuf);
        m_UploadBatchCubeMaps.push_back(cm);
        return cm;
    }

    Ref<CubeMap> VulkanRenderAPI::CreateCubeMapFromEquirectangular(Ref<Texture> equirect, uint faceSize,
                                                                     AssetManager& assets)
    {
        DD_ERR("CreateCubeMapFromEquirectangular is not yet implemented for the Vulkan backend.");
        return nullptr;
    }

    Ref<FrameBuffer> VulkanRenderAPI::CreateFrameBuffer(const FrameBufferProperties& props)
    {
        return std::make_shared<VulkanFrameBuffer>(props, m_Device, m_VmaAllocator);
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
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            bool graphics = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
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

        // MoltenVK on MacOS requires this extension. The extension is not expected for renderdoc since it does not support MacOS, therefore it most be disabled in that case
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

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevice, &props);

        if (VK_API_VERSION_MINOR(props.apiVersion) < 3) {
            // Dynamic rendering is built in core in 1.3+, otherwise we need the extension
            extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VulkanRenderAPI::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                                      const std::vector<const char*>& requiredExtensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        DD_INFO("Vulkan Device Extensions:");
        // Build a hash set of available extension names
        std::unordered_set<std::string> availableExtensionNames;
        availableExtensionNames.reserve(extensionCount);
        for (const auto& extension : availableExtensions) {
            availableExtensionNames.insert(extension.extensionName);
            DD_INFO("{}", extension.extensionName);
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

    /**
     * Select the best surface format given available pixel formats
     */
    VkSurfaceFormatKHR VulkanRenderAPI::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        // TODO: Add a IsBetterSurfaceFormat function to select the best available format
        return availableFormats[0];
    }

    /**
     * Select the best swap present mode given available present modes
     */
    VkPresentModeKHR VulkanRenderAPI::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        // TODO: We want to choose this based on hardware. For example on pcs we want to use MAILBOX but on mobile
        // devices or laptops we want to avoid MAILBOX since it consumes a lot of battery See
        // https://youtu.be/0OqJtPnkfC8?si=Bi7aUphwI486H_Ba&t=1200
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
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