#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo::Platform {

    class OpenGLCubeMap {
      public:
        OpenGLCubeMap(const std::vector<std::string>& paths);
        ~OpenGLCubeMap();

        uint GetTextureID() const { return m_TextureID; }

      private:
        uint m_TextureID;
        uint m_Index;
    };
} // namespace Dodo::Platform