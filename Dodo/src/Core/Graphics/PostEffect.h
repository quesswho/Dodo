#pragma once

#include <Core/Common.h>

#include "Core/Application/Application.h"

#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/Pipeline/Pipeline.h"

namespace Dodo {
    class PostEffect {
      private:
        VertexBuffer* m_Vertexbuffer;
        FrameBuffer* m_Framebuffer;
        Ref<Pipeline> m_Shader;

      public:
        PostEffect(const FrameBufferProperties& framebufferprop, const char* path);
        ~PostEffect();

        inline void Bind() const { m_Framebuffer->Bind(); }

        template <typename T>
        void SetUniformValue(const char* location, const T val, RenderAPI& renderAPI)
        {
            renderAPI.BindPipeline(m_Shader);
            m_Shader->SetUniformValue(location, val);
        }

        void Draw(RenderAPI& renderAPI) const;

        void Resize(uint width, uint height) { m_Framebuffer->Resize(width, height); }

      private:
        void Create();
    };
} // namespace Dodo
