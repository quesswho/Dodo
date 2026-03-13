#include "OpenGLTexture.h"
#include "pch.h"

#include "Core/Math/MathFunc.h"

#include <glad/gl.h>
#include <stb_image.h>

namespace Dodo::Platform {

    OpenGLTexture::OpenGLTexture(const std::string& path) : m_TextureID(0)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        uchar* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (data) {
            m_TextureProperties.m_Width = width;
            m_TextureProperties.m_Height = height;

            switch (channels) {
            case 1:
                m_TextureProperties.m_Format = TextureFormat::FORMAT_RED;
                break;
            case 3:
                m_TextureProperties.m_Format = TextureFormat::FORMAT_RGB;
                break;
            case 4:
                m_TextureProperties.m_Format = TextureFormat::FORMAT_RGBA;
                break;
            default:
                DD_ERR("File format is not supported! {}", path);
            }
            Init(data);
        } else {
            DD_ERR("Could not load texture: {}", path);
        }
        stbi_image_free(data);
    }

    OpenGLTexture::OpenGLTexture(uchar* data, const TextureProperties& prop) : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data);
    }

    void OpenGLTexture::Init(uchar* data)
    {
        GLenum internalFormat, format;
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
        default:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        }

        int mipLevels =
            1 + (int)floor(log2((double)std::max(m_TextureProperties.m_Width, m_TextureProperties.m_Height)));

        glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
        glTextureStorage2D(m_TextureID, mipLevels, internalFormat, m_TextureProperties.m_Width,
                           m_TextureProperties.m_Height);
        glTextureSubImage2D(m_TextureID, 0, 0, 0, m_TextureProperties.m_Width, m_TextureProperties.m_Height, format,
                            GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(m_TextureID);
    }

    OpenGLTexture::~OpenGLTexture()
    {
        glDeleteTextures(1, &m_TextureID);
    }
} // namespace Dodo::Platform