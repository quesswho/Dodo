#include "VkRenderAPI.h"
#include "pch.h"

#include <backends/imgui_impl_vulkan.h>
#include <unordered_set>

namespace Dodo::Platform {

    VkRenderAPI::VkRenderAPI(const NativeWindowHandle& handle) : m_Handle(handle)
    {
#ifdef DD_DEBUG
        m_EnableValidationLayers = true;
        m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#else
        m_EnableValidationLayers = false;
#endif
    }

    VkRenderAPI::~VkRenderAPI()
    {
        vkDestroyDevice(m_Device, nullptr);
        if (m_EnableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    RenderInitError VkRenderAPI::Init(const WindowProperties& winprop)
    {
        RenderInitError result = RenderInitError(RenderInitStatus::Success);

        m_Context.CreateContextImpl(m_Handle);

        // Preload Vulkan using volk
        if (volkInitialize() != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "Glad: Unable to load Vulkan symbols!");
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
        if (winprop.m_Settings.imgui)
            if (Try(InitImGui())) return result;

        return result;
    }

    RenderInitError VkRenderAPI::InitInstance()
    {
        if (m_EnableValidationLayers && !CheckValidationLayerSupport())
            return RenderInitError(RenderInitStatus::Failed, "validation layers requested, but not available!");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dodo Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Add portability enumeration flag for platforms that require it (e.g. macOS with MoltenVK)
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
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

    RenderInitError VkRenderAPI::PickPhysicalDevice()
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

            if (isDeviceBetter(bestDevice, deviceInfo)) {
                m_PhysicalDevice = device;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            return RenderInitError(RenderInitStatus::Failed, "failed to find a suitable GPU!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VkRenderAPI::InitDevice()
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

        // We will specify device features here later
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = 0;

        if (m_EnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to create logical device!");
        }

        vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

        return RenderInitError(RenderInitStatus::Success);
    }

    RenderInitError VkRenderAPI::InitImGui()
    {
        m_Context.InitializeImGui();
        // ImGui_ImplVulkan_Init();

        return RenderInitError(RenderInitStatus::Success);
    }

    bool VkRenderAPI::isDeviceBetter(PhyisicalDeviceInfo bestDevice, PhyisicalDeviceInfo device)
    {
        if (bestDevice.device == VK_NULL_HANDLE) return true;

        if (bestDevice.properties.deviceType < bestDevice.properties.deviceType) {
            /**
             * VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
             * VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
             * VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
             * VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
             * VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
             *
             * Prefer higher valued devices with the exception of CPU devices
             */
            return bestDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU;
        }

        if (device.features.geometryShader != device.features.geometryShader) return device.features.geometryShader;

        if (device.indices.IsComplete() && !bestDevice.indices.IsComplete()) return true;

        // TODO: Selection based on VRAM:
        // https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
        return false;
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

    QueueFamilyIndices VkRenderAPI::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            // Exit early if all required families are found
            if (indices.IsComplete()) break;

            i++;
        }

        return indices;
    }

    std::vector<const char*> VkRenderAPI::GetRequiredExtensions()
    {
        std::vector<const char*> extensions = m_Context.GetExtensions();

        if (m_EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        return extensions;
    }

    bool VkRenderAPI::CheckValidationLayerSupport()
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

        for (const char* layerName : m_ValidationLayers) {
            if (availableLayerNames.find(layerName) == availableLayerNames.end()) {
                DD_ERR("Validation layer not found: {}", layerName);
                return false;
            }
        }
        return true;
    }

    RenderInitError VkRenderAPI::SetupDebug()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            return RenderInitError(RenderInitStatus::Failed, "failed to set up debug messenger!");
        }

        return RenderInitError(RenderInitStatus::Success);
    }

    /**
     * This function populates a VkDebugUtilsMessengerCreateInfoEXT structure with the desired settings for the debug
     * messenger. This function is used while setting up the debug messenger and when creating the Vulkan instance. This
     * ensures that the debug messenger is set up even during instance creation.
     */
    void VkRenderAPI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

    VkResult VkRenderAPI::CreateDebugUtilsMessengerEXT(VkInstance instance,
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

    void VkRenderAPI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                    const VkAllocationCallbacks* pAllocator)
    {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VkRenderAPI::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData)
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