#include "OpenGLTexture.h"

#include "Core/Math/MathFunc.h"

#include <glad/gl.h>
#include <vector>

namespace Dodo::Platform {

    OpenGLTexture::OpenGLTexture(uchar* data, const TextureProperties& prop) : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data);
    }

    void OpenGLTexture::Init(uchar* data)
    {
        GLenum internalFormat, format, type = GL_UNSIGNED_BYTE;
        switch (m_TextureProperties.m_Format) {
        case TextureFormat::FORMAT_RED:
            internalFormat = GL_R8;
            format = GL_RED;
            break;
        case TextureFormat::FORMAT_RGB:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        case TextureFormat::FORMAT_RGBA:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            break;
        case TextureFormat::FORMAT_RGB16F:
            // Stores half-precision floats on the GPU. Upload type is still GL_FLOAT because
            // stbi_loadf() gives 32-bit data: OpenGL converts it to 16-bit on upload.
            internalFormat = GL_RGB16F;
            format = GL_RGB;
            type = GL_FLOAT;
            break;
        case TextureFormat::FORMAT_RGB32F:
            // Upload type is GL_FLOAT because stbi_loadf() gives 32-bit float data per channel.
            internalFormat = GL_RGB32F;
            format = GL_RGB;
            type = GL_FLOAT;
            break;
        default:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        }

        int mipLevels =
            1 + (int)floor(log2((double)std::max(m_TextureProperties.m_Width, m_TextureProperties.m_Height)));

        size_t bpp = 0;
        switch (m_TextureProperties.m_Format) {
        case TextureFormat::FORMAT_RED:
            bpp = 1;
            break;
        case TextureFormat::FORMAT_RGB:
            bpp = 3;
            break;
        case TextureFormat::FORMAT_RGBA:
            bpp = 4;
            break;
        case TextureFormat::FORMAT_RGB16F:
            bpp = 12;
            break;
        case TextureFormat::FORMAT_RGB32F:
            bpp = 12;
            break;
        default:
            break;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
        glTextureStorage2D(m_TextureID, mipLevels, internalFormat, m_TextureProperties.m_Width,
                           m_TextureProperties.m_Height);
        glTextureSubImage2D(m_TextureID, 0, 0, 0, m_TextureProperties.m_Width, m_TextureProperties.m_Height, format,
                            type, data);
        glGenerateTextureMipmap(m_TextureID);
    }

    OpenGLTexture::~OpenGLTexture()
    {
        glDeleteTextures(1, &m_TextureID);
    }
} // namespace Dodo::Platform