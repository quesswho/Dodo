#pragma once

#include <Core/Common.h>

#include "Core/Application/Application.h"

#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"

namespace Dodo {
    class PostEffect {
      private:
        Ref<VertexBuffer> m_Vertexbuffer;
        Ref<FrameBuffer> m_Framebuffer;
        Ref<Pipeline> m_Shader;
        Ref<TextureSampler> m_Sampler;
        std::vector<uint8_t> m_PushConstantData;

      public:
        PostEffect(const FrameBufferProperties& framebufferprop, const char* path, RenderAPI& renderAPI,
                   AssetManager& assets);
        ~PostEffect();

        inline void Bind() const { m_Framebuffer->Bind(); }

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

        void Draw(RenderAPI& renderAPI) const;

        void Resize(uint width, uint height) { m_Framebuffer->Resize(width, height); }
    };
} // namespace Dodo
