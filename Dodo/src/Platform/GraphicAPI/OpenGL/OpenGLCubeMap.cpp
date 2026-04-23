#include "OpenGLCubeMap.h"

#include "Core/Utilities/Logger.h"

#include <cmath>
#include <glad/gl.h>

namespace Dodo::Platform {

    OpenGLCubeMap::OpenGLCubeMap(const CubeMapData& data) : m_TextureID(0)
    {
        if (data.faces[0].pixels.empty()) {
            DD_ERR("OpenGLCubeMap: CubeMapData is empty (load failed)");
            return;
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);

        const TextureProperties& props = data.faces[0].props;
        const int width = (int)props.m_Width;
        const int height = (int)props.m_Height;
        const int mipLevels = 1 + (int)std::floor(std::log2((double)std::max(width, height)));

        // All faces share the same format (RGBA forced by CubeMapLoader)
        glTextureStorage2D(m_TextureID, mipLevels, GL_RGBA8, width, height);

        for (int i = 0; i < 6; i++) {
            glTextureSubImage3D(m_TextureID, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                data.faces[i].pixels.data());
        }

        glGenerateTextureMipmap(m_TextureID);
    }

    OpenGLCubeMap::OpenGLCubeMap(uint existingTextureID) : m_TextureID(existingTextureID) {}

    OpenGLCubeMap::~OpenGLCubeMap()
    {
        glDeleteTextures(1, &m_TextureID);
    }
} // namespace Dodo::Platform
