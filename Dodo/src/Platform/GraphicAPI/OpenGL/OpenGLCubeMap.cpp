#include "OpenGLCubeMap.h"
#include "pch.h"

#include <glad/gl.h>
#include <stb_image.h>

namespace Dodo::Platform {

    OpenGLCubeMap::OpenGLCubeMap(const std::vector<std::string>& paths) : m_TextureID(0)
    {
        if (paths.size() != 6) {
            DD_ERR("CubeMap requires exactly 6 faces, got {}", paths.size());
            return;
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);

        for (int i = 0; i < paths.size(); i++) {
            int channels, width, height;
            stbi_set_flip_vertically_on_load(false);
            uchar* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);
            if (!data) {
                DD_ERR("Could not load cubemap face: {}", paths[i]);
                return;
            }
            GLenum format, internalFormat;
            switch (channels) {
            case 3:
                format = GL_RGB;
                internalFormat = GL_RGB8;
                break;
            case 4:
                format = GL_RGBA;
                internalFormat = GL_RGBA8;
                break;
            default:
                DD_ERR("Unsupported channel on face {}: {}", i, paths[i]);
                stbi_image_free(data);
                return;
            }
            // Allocate storage based on first face
            if (i == 0) {
                // TODO: Store mip map level in texture settings and use it here
                // int mipLevels = 1 + (int)floor(log2((double)std::max(width, height)));
                int mipLevels = 1;
                glTextureStorage2D(m_TextureID, mipLevels, internalFormat, width, height);
            }

            glTextureSubImage3D(m_TextureID, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }

        // glGenerateTextureMipmap(m_TextureID);
    }

    OpenGLCubeMap::~OpenGLCubeMap()
    {
        glDeleteTextures(1, &m_TextureID);
    }
} // namespace Dodo::Platform