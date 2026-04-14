#include "TextureLoader.h"
#include "pch.h"

#include <stb_image.h>

namespace Dodo {

    TextureData TextureLoader::Load(const std::string& path)
    {
        TextureData result;
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);

        if (stbi_is_hdr(path.c_str())) {
            float* hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
            if (!hdrData) {
                DD_ERR("TextureLoader: could not load HDR '{}'", path);
                return result;
            }
            result.props.m_Width = (uint)width;
            result.props.m_Height = (uint)height;
            result.props.m_Format = TextureFormat::FORMAT_RGB16F;
            size_t byteCount = (size_t)width * height * 3 * sizeof(float);
            result.pixels.resize(byteCount);
            memcpy(result.pixels.data(), hdrData, byteCount);
            stbi_image_free(hdrData);
            DD_INFO("TextureLoader: finished loading HDR texture to CPU '{}'", path);
            return result;
        }

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
