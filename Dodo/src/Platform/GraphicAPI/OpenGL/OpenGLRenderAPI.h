#pragma once

#include "Core/Application/WindowProperties.h"
#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/FrameBufferedDescriptorSet.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"
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

namespace Dodo {
    class AssetManager;
}

namespace Dodo::Platform {
    class OpenGLRenderAPI {
      public:
        OpenGLRenderAPI(const NativeWindowHandle& NativeWindowHandle);
        ~OpenGLRenderAPI();
        RenderInitError Init(const WindowProperties& winprop);

        inline void Begin() const { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
        inline void End() { m_Context.SwapBuffer(); }

        void WaitIdle() const { glFinish(); }

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
        inline void BindFrameBufferTexture(uint slot, Ref<FrameBuffer> framebuffer)
        {
            glBindSampler(slot, framebuffer->m_Sampler->GetSamplerID());
            glBindTextureUnit(slot, framebuffer->m_TextureID);
        }

        void BindPipeline(Ref<Pipeline> pipeline);
        void SetMaterialDescriptorSet(FrameBufferedDescriptorSet&) {}
        void PushConstants(const void* data, size_t size);
        void SetFrameData(const FrameData& data);
        void SetDrawData(const DrawData& data);
        inline void SetCSMData(const CsmData& data)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, m_LightSpaceUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CsmData), &data);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        void DrawIndexed(const Ref<VertexBuffer>& va);
        inline void DrawIndices(uint count) const { glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0); }
        inline void DrawIndicesInstanced(uint count, uint instanceCount) const
        {
            glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0, instanceCount);
        }
        inline void DrawArray(uint count) const { glDrawArrays(GL_TRIANGLES, 0, count); }
        void DefaultFrameBuffer() const;
        void SetViewport(uint width, uint height);
        void SetViewport(uint width, uint height, uint posX, uint posY);

        inline void BindVertexBuffer(const Ref<VertexBuffer>& vb) { glBindVertexArray(vb->GetVAOID()); }
        inline void BindIndexBuffer(const Ref<IndexBuffer>& ib)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->GetEBOID());
        }

        Ref<Pipeline> CreatePipeline(const PipelineDesc& desc, AssetManager& assets);
        inline Ref<VertexBuffer> CreateVertexBuffer(const float* v, uint size, const BufferProperties& prop)
        {
            return std::make_shared<VertexBuffer>(v, size, prop);
        }
        inline Ref<IndexBuffer> CreateIndexBuffer(const uint* i, uint count)
        {
            return std::make_shared<IndexBuffer>(i, count);
        }
        inline Ref<Texture> CreateTexture(uchar* data, const TextureProperties& prop)
        {
            return std::make_shared<Texture>(data, prop);
        }
        inline Ref<TextureSampler> CreateSampler(const SamplerProperties& prop)
        {
            return std::make_shared<TextureSampler>(prop);
        }
        inline Ref<CubeMap> CreateCubeMap(const CubeMapData& data) { return std::make_shared<CubeMap>(data); }

        // OpenGL uploads are synchronous so these are no-ops / always-ready stubs.
        inline void SubmitTextureBatch() {}
        inline bool PollTextureBatch() { return true; }
        Ref<CubeMap> CreateCubeMapFromEquirectangular(Ref<Texture> equirect, uint faceSize, AssetManager& assets);
        inline Ref<FrameBuffer> CreateFrameBuffer(const FrameBufferProperties& props)
        {
            return std::make_shared<FrameBuffer>(props);
        }
        inline void BindFrameBuffer(Ref<FrameBuffer> framebuffer) { framebuffer->Bind(); }

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
        struct DrawDataUBO { // This struct is padded to stb140
            Math::Mat4 model;
            Math::Mat4 normalMatrix; // Note: This is a padded mat3
        };

        uint m_CurrentPipelineID;
        Ref<Pipeline> m_CurrentPipeline;
        uint m_FrameUBO = 0;
        uint m_ModelUBO = 0;
        uint m_PushConstantUBO = 0;
        uint m_LightSpaceUBO = 0;

        bool m_ImGuiLoaded = false;

        int m_Version;
        NativeWindowHandle m_Handle;
    };
} // namespace Dodo::Platform
