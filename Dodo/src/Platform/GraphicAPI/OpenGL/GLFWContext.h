#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <Platform/WindowAPI/NativeWindowHandle.h>

namespace Dodo::Platform {

class GLFWContext {
public:
    explicit GLFWContext() {}

    void CreateContextImpl(const NativeWindowHandle& handle) {
        m_Window = reinterpret_cast<GLFWwindow*>(handle.window);
        glfwMakeContextCurrent(m_Window);
    }

    int LoadGlad() { return gladLoadGL(glfwGetProcAddress); }
    void SwapBuffer() { glfwSwapBuffers(m_Window); }
    void SetVSync(bool enabled) { glfwSwapInterval(enabled ? 1 : 0); }

private:
    GLFWwindow* m_Window = nullptr;
};

} // namespace
