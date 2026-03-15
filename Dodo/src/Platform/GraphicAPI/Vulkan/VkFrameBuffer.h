#pragma once

#include "Core/Graphics/FrameBufferProperties.h"
#include <Core/Common.h>

namespace Dodo::Platform {
    class VkFrameBuffer {
      private:
        uint m_FrameBufferID, m_TextureID, m_RenderBuffer;
        FrameBufferProperties m_FrameBufferProperties;

      public:
        VkFrameBuffer(const FrameBufferProperties& framebufferprop);
        ~VkFrameBuffer();

        inline void Bind() const {}

        inline void BindTexture(uint index = 0) const {}

        void Resize(uint width, uint height);
        inline uint GetTextureHandle() { return m_TextureID; }

      private:
        void Create();
    };
} // namespace Dodo::Platform