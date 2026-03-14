#include "VkTexture.h"
#include "pch.h"

#include <stb_image.h>

namespace Dodo::Platform {

    VkTexture::VkTexture(const std::string& path)
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

    VkTexture::VkTexture(uchar* data, const TextureProperties& prop) : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data);
    }

    void VkTexture::Init(uchar* data) {}

    VkTexture::~VkTexture() {}
} // namespace Dodo::Platform