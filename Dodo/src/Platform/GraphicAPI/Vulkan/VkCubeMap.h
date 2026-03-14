#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo { namespace Platform {

    class VkCubeMapTexture {
      private:
        uint m_TextureID;

      public:
        VkCubeMapTexture(std::vector<std::string> paths, uint index = 0,
                         const TextureSettings& prop = TextureSettings());
        ~VkCubeMapTexture();

        void Bind() const;

      public:
        uint m_Index;
    };
}} // namespace Dodo::Platform