#pragma once
#include <Windows.h>

extern "C" {
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

namespace Dodo::Platform {

    class Win32ImGuiBackend {
      public:
        static void Init(HWND hwnd, bool enableDocking = false);
        static void Shutdown();
        static void NewFrame();
        static void EndFrame();
        static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
} // namespace Dodo::Platform