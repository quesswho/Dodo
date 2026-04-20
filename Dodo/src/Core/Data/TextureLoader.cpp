#include "TextureLoader.h"
#include "pch.h"

#include <stb_image.h>

namespace Dodo {

    TextureData TextureLoader::Load(const std::string& path)
    {
        return stbi_is_hdr(path.c_str()) ? LoadHDR(path) : LoadLDR(path);
    }

    TextureData TextureLoader::LoadHDR(const std::string& path)
    {
        TextureData result;
        int width, height, channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
        if (!data) {
            DD_ERR("TextureLoader: could not load HDR '{}'", path);
            return result;
        }

        result.props.m_Width = (uint)width;
        result.props.m_Height = (uint)height;
        // The typical industry standard is half float precision.
        // We might consider adding support for RGBA
        result.props.m_Format = TextureFormat::FORMAT_RGB16F; 

        
        const size_t byteCount = (size_t)width * height * 3 * sizeof(float);
        result.pixels.resize(byteCount);
        memcpy(result.pixels.data(), data, byteCount);
        stbi_image_free(data);

        DD_INFO("TextureLoader: finished loading HDR texture to CPU '{}'", path);
        return result;
    }

    TextureData TextureLoader::LoadLDR(const std::string& path)
    {
        TextureData result;
        int width, height, channels;
        uchar* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) {
            DD_ERR("TextureLoader: could not load '{}'", path);
            return result;
        }

        result.props.m_Width = (uint)width;
        result.props.m_Height = (uint)height;

        switch (channels) {
        case 1:
            result.props.m_Format = TextureFormat::FORMAT_RED;
            break;
        case 3:
            result.props.m_Format = TextureFormat::FORMAT_RGB;
            break;
        case 4:
            result.props.m_Format = TextureFormat::FORMAT_RGBA;
            break;
        default:
            DD_ERR("TextureLoader: unsupported channel count {} in '{}'", channels, path);
            stbi_image_free(data);
            return result;
        }

        result.pixels.assign(data, data + (size_t)width * height * channels);
        stbi_image_free(data);

        DD_INFO("TextureLoader: finished loading texture to CPU '{}'", path);
        return result;
    }

} // namespace Dodo
