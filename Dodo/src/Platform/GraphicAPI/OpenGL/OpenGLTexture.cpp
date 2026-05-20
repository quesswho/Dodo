#include "OpenGLTexture.h"

#include "Core/Math/MathFunc.h"

#include <glad/gl.h>
#include <vector>

namespace Dodo::Platform {

    static bool IsCompressedFormat(TextureFormat fmt)
    {
        switch (fmt) {
        case TextureFormat::FORMAT_BC1_RGB_UNORM:
        case TextureFormat::FORMAT_BC3_RGBA_UNORM:
        case TextureFormat::FORMAT_BC5_RG_UNORM:
        case TextureFormat::FORMAT_BC7_RGBA_UNORM:
            return true;
        default:
            return false;
        }
    }

    static GLenum CompressedInternalFormat(TextureFormat fmt)
    {
        switch (fmt) {
        case TextureFormat::FORMAT_BC1_RGB_UNORM:  return 0x83F0; // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        case TextureFormat::FORMAT_BC3_RGBA_UNORM: return 0x83F3; // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        case TextureFormat::FORMAT_BC5_RG_UNORM:   return 0x8DBD; // GL_COMPRESSED_RG_RGTC2
        case TextureFormat::FORMAT_BC7_RGBA_UNORM: return 0x8E8C; // GL_COMPRESSED_RGBA_BPTC_UNORM
        default:                                   return 0;
        }
    }

    static size_t CompressedMipBytes(TextureFormat fmt, uint32_t w, uint32_t h)
    {
        uint32_t blockBytes = (fmt == TextureFormat::FORMAT_BC1_RGB_UNORM) ? 8u : 16u;
        return ((w + 3u) / 4u) * ((h + 3u) / 4u) * blockBytes;
    }

    OpenGLTexture::OpenGLTexture(uchar* data, const TextureProperties& prop)
        : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data, {});
    }

    OpenGLTexture::OpenGLTexture(const uchar* data, const std::vector<size_t>& mipOffsets,
                                 const TextureProperties& prop)
        : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data, mipOffsets);
    }

    void OpenGLTexture::Init(const uchar* data, const std::vector<size_t>& mipOffsets)
    {
        if (IsCompressedFormat(m_TextureProperties.m_Format)) {
            GLenum fmt = CompressedInternalFormat(m_TextureProperties.m_Format);
            uint32_t levels = m_TextureProperties.m_MipLevels;

            glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
            glTextureStorage2D(m_TextureID, (GLsizei)levels, fmt,
                               (GLsizei)m_TextureProperties.m_Width,
                               (GLsizei)m_TextureProperties.m_Height);

            for (uint32_t i = 0; i < levels; ++i) {
                uint32_t w = std::max(1u, m_TextureProperties.m_Width  >> i);
                uint32_t h = std::max(1u, m_TextureProperties.m_Height >> i);
                GLsizei mipBytes = (GLsizei)CompressedMipBytes(m_TextureProperties.m_Format, w, h);
                glCompressedTextureSubImage2D(m_TextureID, (GLint)i, 0, 0,
                                             (GLsizei)w, (GLsizei)h,
                                             fmt, mipBytes,
                                             data + mipOffsets[i]);
            }
            return;
        }

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