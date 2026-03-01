#pragma once

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <Platform/WindowAPI/NativeWindowHandle.h>

#include <backends/imgui_impl_glfw.h>

namespace Dodo::Platform {

    class GLFWContext {
      public:
        explicit GLFWContext()
        {}

        void CreateContextImpl(const NativeWindowHandle &handle)
        {
            m_Window = reinterpret_cast<GLFWwindow *>(handle.window);
            glfwMakeContextCurrent(m_Window);
        }

        int LoadGlad()
        {
            return gladLoadGL(glfwGetProcAddress);
        }
        void SwapBuffer()
        {
            glfwSwapBuffers(m_Window);
        }
        void SetVSync(bool enabled)
        {
            glfwMakeContextCurrent(m_Window);
            glfwSwapInterval(enabled ? 1 : 0);
        }

        void InitializeImGui()
        {
            ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        }

      private:
        GLFWwindow *m_Window = nullptr;
    };

} // namespace Dodo::Platform
