#pragma once

#include <Core/Common.h>

#include "Core/Application/Application.h"

#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/FrameBufferedDescriptorSet.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"

namespace Dodo {
    class PostEffect {
      private:
        Ref<VertexBuffer> m_Vertexbuffer;
        Ref<FrameBuffer> m_Framebuffer;
        Ref<FrameBuffer> m_Output;
        Ref<Pipeline> m_Shader;
        Ref<TextureSampler> m_Sampler;
        std::vector<uint8_t> m_PushConstantData;
        mutable FrameBufferedDescriptorSet m_MaterialSet;

      public:
        /**
         * @param framebufferprop Properties of the framebuffer the scene is rendered into before the effect.
         * @param path            Path of the effect shader.
         * @param output          Framebuffer the effect result is drawn into, or nullptr for the swapchain.
         */
        PostEffect(const FrameBufferProperties& framebufferprop, const char* path, RenderAPI& renderAPI,
                   AssetManager& assets, Ref<FrameBuffer> output = nullptr);
        ~PostEffect();

        inline void Bind(RenderAPI& renderAPI) { renderAPI.BindFrameBuffer(m_Framebuffer); }

        /**
         * Set data for the post effect shader.
         */
        template <typename T>
        void SetEffectData(const T& data)
        {
            static_assert(
                sizeof(T) <= 128,
                "Push constant data exceeds 128 byte limit!"); // This is bounded by vulkans guaranteed min of 128
                                                               // bytes. As of 2026, about 29% of devices use this
                                                               // limit:
                                                               // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPushConstantsSize&platform=all
            m_PushConstantData.resize(sizeof(T));
            memcpy(m_PushConstantData.data(), &data, sizeof(T));
        }

        /**
         * Draws the effect into the output framebuffer, or the swapchain if there is none. The target stays
         * bound afterwards.
         */
        void Draw(RenderAPI& renderAPI) const;

        void Resize(uint width, uint height) { m_Framebuffer->Resize(width, height); }
    };
} // namespace Dodo
