#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo::Platform {
    class VkTexture {
      public:
        VkTexture(const std::string& path);
        VkTexture(uchar* data, const TextureProperties& prop);
        ~VkTexture();

        uint GetTextureID() const { return m_TextureID; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }

      private:
        void Init(uchar* data);

        TextureProperties m_TextureProperties;
        uint m_TextureID;
    };
} // namespace Dodo::Platform