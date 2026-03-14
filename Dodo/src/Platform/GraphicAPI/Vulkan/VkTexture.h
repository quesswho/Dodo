#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo { namespace Platform {

    class VkTexture {
      private:
        uint m_TextureID;

      public:
        VkTexture(const std::string& path, uint index = 0, const TextureSettings& settings = TextureSettings());
        VkTexture(uchar* data, TextureProperties prop, uint index = 0,
                  const TextureSettings& settings = TextureSettings());
        ~VkTexture();

        void Bind() const;

      private:
        void Init(uchar* data, const TextureSettings& settings);

      public:
        uint m_Index;
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }

      private:
        TextureProperties m_TextureProperties;
    };
}} // namespace Dodo::Platform