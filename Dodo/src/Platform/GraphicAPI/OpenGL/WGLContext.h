#pragma once

#include <glad/gl.h>
#include <Platform/WindowAPI/NativeWindowHandle.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Dodo::Platform {
    class WGLContext{
        public:
            explicit WGLContext() {}

            void CreateContextImpl(const NativeWindowHandle& handle) {
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
                
                gladLoaderLoadWGL(m_Hdc);
            }

            void MakeCurrentImpl() { wglMakeCurrent(m_Hdc, m_Hglrc); }
            void SwapBuffersImpl() { ::SwapBuffers(m_Hdc); }
            void SetVSyncImpl(bool enabled) { wglSwapIntervalEXT(enabled ? 1 : 0); }

        private:
            HDC   m_Hdc = nullptr;
            HGLRC m_Hglrc = nullptr;
    };
}