#pragma once

#include <GLFW/glfw3.h>
#include <glad/vulkan.h>

#include <Platform/WindowAPI/NativeWindowHandle.h>

#include <backends/imgui_impl_glfw.h>

namespace Dodo::Platform {

    class VulkanGLFWContext {
      public:
        explicit VulkanGLFWContext() {}

        void CreateContextImpl(const NativeWindowHandle& handle)
        {
            m_Window = reinterpret_cast<GLFWwindow*>(handle.window);
            glfwMakeContextCurrent(m_Window);
        }
        void SwapBuffer() { glfwSwapBuffers(m_Window); }
        void SetVSync(bool enabled)
        {
            glfwMakeContextCurrent(m_Window);
            glfwSwapInterval(enabled ? 1 : 0);
        }

        void InitializeImGui() { ImGui_ImplGlfw_InitForVulkan(m_Window, true); }

      private:
        GLFWwindow* m_Window = nullptr;
    };

} // namespace Dodo::Platform
