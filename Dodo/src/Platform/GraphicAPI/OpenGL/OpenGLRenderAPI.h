#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Core/Graphics/Pipeline/ShaderSource.h"
#include "Core/Graphics/RenderAPITypes.h"

#include <glad/gl.h>

#include <Platform/WindowAPI/NativeWindowHandle.h>
#ifdef DD_API_WIN32
#include "WGLContext.h"
using OpenGLContext = Dodo::Platform::WGLContext;
#elif defined(DD_API_GLFW)
#include "GLFWContext.h"
using OpenGLContext = Dodo::Platform::GLFWContext;
#endif

namespace Dodo::Platform {
    class OpenGLRenderAPI {
      public:
        OpenGLRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~OpenGLRenderAPI();
        RenderInitError Init(const WindowProperties& winprop);

        inline void Begin() const { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
        inline void End() { m_Context.SwapBuffer(); }

        void ImGuiNewFrame() const;
        void ImGuiEndFrame() const;

        inline void ClearColor(float r, float g, float b) const { glClearColor(r, g, b, 1.0f); }
        inline void Viewport(uint width, uint height) const { glViewport(0, 0, (GLsizei)width, (GLsizei)height); }

        inline void BindCubeMap(uint slot, Ref<CubeMap> cubemap) { glBindTextureUnit(slot, cubemap->GetTextureID()); }
        inline void BindTexture(uint slot, Ref<Texture> texture) { glBindTextureUnit(slot, texture->GetTextureID()); }
        inline void BindTextureSampler(uint slot, Ref<TextureSampler> sampler)
        {
            glBindSampler(slot, sampler->GetSamplerID());
        }
        void BindPipeline(Ref<Pipeline> pipeline);
        void SetFrameData(const FrameData& data);
        void SetDrawData(const DrawData& data);
        void DrawIndexed(const Ref<VertexBuffer>& va);
        inline void DrawIndices(uint count) const { glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0); }
        inline void DrawArray(uint count) const { glDrawArrays(GL_TRIANGLES, 0, count); }
        void DefaultFrameBuffer() const;
        void SetViewport(uint width, uint height);
        void SetViewport(uint width, uint height, uint posX, uint posY);

        Ref<Pipeline> CreatePipeline(const PipelineDesc& desc, const ShaderSource& source);

        inline const char* GetAPIName() const { return "OpenGL"; }
        int CurrentVRamUsage() const
        {
            int availKb;
            glGetIntegerv(0x9049, &availKb); // Current available

            return m_VramKbs - availKb;
        }

        OpenGLContext m_Context;

        std::string m_GPUInfo;
        int m_VramKbs;

        uint m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY;

        bool m_CullingDefault;

      private:
        uint m_CurrentPipelineID;
        uint m_FrameUBO = 0;

        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform
