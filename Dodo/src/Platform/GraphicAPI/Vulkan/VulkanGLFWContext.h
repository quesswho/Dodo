#pragma once

// Note: Volk needs to be included before GLFW
#include <volk.h>

#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>

#include "Platform/WindowAPI/NativeWindowHandle.h"

#include <vector>

namespace Dodo::Platform {
    class VulkanGLFWContext {
      public:
        explicit VulkanGLFWContext() {}

        void CreateContextImpl(const NativeWindowHandle& handle)
        {
            m_Window = reinterpret_cast<GLFWwindow*>(handle.window);
        }

        std::vector<const char*> GetExtensions()
        {
            std::vector<const char*> requiredExtensions;
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }
            return requiredExtensions;
        }

        bool CreateSurface(VkInstance instance, VkSurfaceKHR* surface)
        {
            return glfwCreateWindowSurface(instance, m_Window, nullptr, surface) == VK_SUCCESS;
        }

        void GetFrameBufferSize(int* width, int* height) { glfwGetFramebufferSize(m_Window, width, height); }

        void InitializeImGui() { ImGui_ImplGlfw_InitForVulkan(m_Window, true); }

      private:
        GLFWwindow* m_Window = nullptr;
    };

} // namespace Dodo::Platform
