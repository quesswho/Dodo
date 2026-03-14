#include "VkTexture.h"
#include "pch.h"

#include "Core/Math/MathFunc.h"

#include <glad/gl.h>
#include <stb_image.h>

namespace Dodo { namespace Platform {

    VkTexture::VkTexture(const std::string& path, uint index, const TextureSettings& settings)
        : m_Index(index), m_TextureID(0)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        uchar* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (data) {
            TextureProperties props((uint)width, (uint)height);
            switch (channels) {
            case 1:
                props.m_Format = TextureFormat::FORMAT_RED;
                break;
            case 3:
                props.m_Format = TextureFormat::FORMAT_RGB;
                break;
            case 4:
                props.m_Format = TextureFormat::FORMAT_RGBA;
                break;
            default:
                DD_ERR("File format is not supported! {}", path);
            }
            m_TextureProperties = props;
            Init(data, settings);
        } else {
            DD_ERR("Could not load texture: {}", path);
        }
        stbi_image_free(data);
    }

    VkTexture::VkTexture(uchar* data, TextureProperties prop, uint index, const TextureSettings& settings)
        : m_Index(index), m_TextureProperties(prop)
    {
        Init(data, settings);
    }

    void VkTexture::Init(uchar* data, const TextureSettings& settings)
    {
        
    }

    VkTexture::~VkTexture()
    {
        
    }

    void VkTexture::Bind() const
    {
    }
}} // namespace Dodo::Platform