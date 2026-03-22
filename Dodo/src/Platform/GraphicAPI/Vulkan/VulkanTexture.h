#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo::Platform {
    class VulkanTexture {
      public:
        VulkanTexture(const std::string& path);
        VulkanTexture(uchar* data, const TextureProperties& prop);
        ~VulkanTexture();

        uint GetTextureID() const { return m_TextureID; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }

      private:
        void Init(uchar* data);

        TextureProperties m_TextureProperties;
        uint m_TextureID;
    };
} // namespace Dodo::Platform