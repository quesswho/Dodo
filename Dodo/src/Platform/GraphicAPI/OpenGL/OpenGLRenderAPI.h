#pragma once

#include "Core/Graphics/Shader/ShaderBuilder.h"

#include "Core/Application/WindowProperties.h"
#include "Core/Graphics/RenderInitResult.h"

#include <glad/gl.h>

#include <Platform/WindowAPI/NativeWindowHandle.h>
#ifdef DD_API_WIN32
#include "WGLContext.h"
using OpenGLContext = Dodo::Platform::WGLContext;
#elif DD_API_GLFW
#include "GLFWContext.h"
using OpenGLContext = Dodo::Platform::GLFWContext;
#endif

namespace Dodo {

    enum class DepthComparisonFunction : uint
    {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
        DEFAULT = LESS
    };

    namespace Platform {

        class OpenGLRenderAPI {
          public:
            OpenGLRenderAPI(const NativeWindowHandle &NativeWindowHandle);
            ~OpenGLRenderAPI();
            RenderInitError Init(const WindowProperties &winprop);

            inline void Begin() const
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }
            inline void End()
            {
                m_Context.SwapBuffer();
            }

            void ImGuiNewFrame() const;
            void ImGuiEndFrame() const;

            inline void ClearColor(float r, float g, float b) const
            {
                glClearColor(r, g, b, 1.0f);
            }
            inline void Viewport(uint width, uint height) const
            {
                glViewport(0, 0, (GLsizei)width, (GLsizei)height);
            }
            inline void DrawIndices(uint count) const
            {
                glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
            }
            inline void DrawArray(uint count) const
            {
                glDrawArrays(GL_TRIANGLES, 0, count);
            }
            void DefaultFrameBuffer() const;
            void ResizeDefaultViewport(uint width, uint height);
            void ResizeDefaultViewport(uint width, uint height, uint posX, uint posY);

            inline void DepthComparisonFunction(DepthComparisonFunction func) const
            {
                glDepthFunc(GL_NEVER + (int)func);
            }
            inline void DepthTest(bool depthtest) const
            {
                depthtest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            }
            inline void StencilTest(bool stenciltest) const
            {
                stenciltest ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
            }
            void Blending(bool blending) const;
            void Culling(bool cull, bool backface = true);

            inline const char *GetAPIName() const
            {
                return "OpenGL";
            }
            int CurrentVRamUsage() const
            {
                int availKb;
                glGetIntegerv(0x9049, &availKb); // Current available

                return m_VramKbs - availKb;
            }

            OpenGLContext m_Context;

            ShaderBuilder *m_ShaderBuilder;

            std::string m_GPUInfo;
            int m_VramKbs;

            uint m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY;

            bool m_CullingDefault;

          private:
            int m_Version;
            NativeWindowHandle m_Handle;
        };
    } // namespace Platform
} // namespace Dodo