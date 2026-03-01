#include "GLFWImGuiBackend.h"

#include <backends/imgui_impl_glfw.h>

namespace Dodo::Platform {

    void GLFWImGuiBackend::Init(GLFWwindow *window, bool enableDocking)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        if (enableDocking)
        {
            ImGuiIO &io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
    }

    void GLFWImGuiBackend::Shutdown()
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void GLFWImGuiBackend::NewFrame()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GLFWImGuiBackend::EndFrame() { ImGui::Render(); }

} // namespace Dodo::Platform