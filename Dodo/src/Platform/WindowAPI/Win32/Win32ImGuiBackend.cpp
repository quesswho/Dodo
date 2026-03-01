#include "Win32UIHook.h"
#include <backends/imgui_impl_win32.h>

namespace Dodo::Platform {

    void Win32ImGuiBackend::Init(HWND hwnd, bool enableDocking)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplWin32_EnableDpiAwareness();

        if (enableDocking)
        {
            ImGuiIO &io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
    }

    void Win32ImGuiBackend::Shutdown()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Win32ImGuiBackend::NewFrame()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Win32ImGuiBackend::EndFrame()
    {
        ImGui::Render();
    }

    LRESULT Win32ImGuiBackend::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    }
} // namespace Dodo::Platform