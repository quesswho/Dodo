#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo::Platform {

    class VkCubeMap {
      private:
        uint m_TextureID;

      public:
        VkCubeMap(std::vector<std::string> paths);
        ~VkCubeMap();

        void Bind() const;

      public:
        uint m_Index;
    };
} // namespace Dodo::Platform