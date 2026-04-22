#pragma once

#include "Core/Data/CubeMapLoader.h"
#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo::Platform {

    class OpenGLCubeMap {
      public:
        OpenGLCubeMap(const CubeMapData& data);
        explicit OpenGLCubeMap(uint existingTextureID);
        ~OpenGLCubeMap();

        uint GetTextureID() const { return m_TextureID; }
        void FinalizeUpload() {} // no-op: OpenGL uploads are synchronous

      private:
        uint m_TextureID;
        uint m_Index;
    };
} // namespace Dodo::Platform