#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo::Platform {

    class VulkanCubeMap {
      private:
        uint m_TextureID;

      public:
        VulkanCubeMap(std::vector<std::string> paths);
        ~VulkanCubeMap();

        void Bind() const;

      public:
        uint m_Index;
    };
} // namespace Dodo::Platform