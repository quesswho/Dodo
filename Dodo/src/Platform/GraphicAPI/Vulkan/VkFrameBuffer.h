#pragma once

#include <Core/Common.h>

#include <glad/gl.h>

namespace Dodo {

    enum class FrameBufferType {
        FRAMEBUFFER_COLOR_DEPTH_STENCIL,
        FRAMEBUFFER_DEPTH
    };

    struct FrameBufferProperties {
        FrameBufferProperties()
            : m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL)
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type)
            : m_Width(width), m_Height(height), m_FrameBufferType(type)
        {}

        uint m_Width, m_Height;
        FrameBufferType m_FrameBufferType;
    };

    namespace Platform {

        class VkFrameBuffer {
          private:
            uint m_FrameBufferID, m_TextureID, m_RenderBuffer;
            FrameBufferProperties m_FrameBufferProperties;

          public:
            VkFrameBuffer(const FrameBufferProperties& framebufferprop);
            ~VkFrameBuffer();

            inline void Bind() const
            {
            }

            inline void BindTexture(uint index = 0) const
            {
            }

            void Resize(uint width, uint height);
            inline uint GetTextureHandle() { return m_TextureID; }

          private:
            void Create();
        };
    } // namespace Platform
} // namespace Dodo