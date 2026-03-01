#pragma once

#include <Platform/WindowAPI/NativeWindowHandle.h>
#include <glad/gl.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <backends/imgui_impl_win32.h>

namespace Dodo::Platform {
    class WGLContext {
      public:
        explicit WGLContext() {}

        void CreateContextImpl(const NativeWindowHandle &handle)
        {
            m_HWND = (HWND)handle.window;
            m_Hdc = (HDC)handle.display;

            PIXELFORMATDESCRIPTOR pfd = {};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;
            pfd.iLayerType = PFD_MAIN_PLANE;

            int pf = ChoosePixelFormat(m_Hdc, &pfd);
            SetPixelFormat(m_Hdc, pf, &pfd);

            m_Hglrc = wglCreateContext(m_Hdc);
            wglMakeCurrent(m_Hdc, m_Hglrc);
        }

        int LoadGlad() { return gladLoaderLoadWGL(m_Hdc); }
        void SwapBuffers() { ::SwapBuffers(m_Hdc); }
        void SetVSync(bool enabled) { wglSwapIntervalEXT(enabled ? 1 : 0); }

        void InitializeImGui() { ImGui_ImplWin32_InitForOpenGL(m_HWND); }

      private:
        HWND m_HWND = nullptr;
        HDC m_Hdc = nullptr;
        HGLRC m_Hglrc = nullptr;
    };
} // namespace Dodo::Platform