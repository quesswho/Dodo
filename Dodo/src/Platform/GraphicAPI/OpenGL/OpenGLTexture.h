#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <vector>

namespace Dodo::Platform {
    class OpenGLTexture {
      public:
        OpenGLTexture(uchar* data, const TextureProperties& prop);
        OpenGLTexture(const uchar* data, const std::vector<size_t>& mipOffsets,
                      const TextureProperties& prop);
        ~OpenGLTexture();

        uint GetTextureID() const { return m_TextureID; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }
        void FinalizeUpload() {} // no-op: OpenGL uploads are synchronous

      private:
        void Init(const uchar* data, const std::vector<size_t>& mipOffsets);

        TextureProperties m_TextureProperties;
        uint m_TextureID;
    };
} // namespace Dodo::Platform