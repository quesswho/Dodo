#include "TextureLoader.h"

#include "Core/Math/MathFunc.h"
#include "Core/Utilities/Logger.h"

#include <stb_image.h>
#include <gli/gli.hpp>

#include <cctype>
#include <cstring>

namespace Dodo {

    TextureData TextureLoader::Load(const std::string& path)
    {
        if (path.size() >= 4) {
            std::string ext = path.substr(path.size() - 4);
            for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
            if (ext == ".dds")
                return LoadDDS(path);
        }
        return stbi_is_hdr(path.c_str()) ? LoadHDR(path) : LoadLDR(path);
    }

    TextureData TextureLoader::LoadDDS(const std::string& path)
    {
        TextureData result;

        gli::texture2d tex(gli::load_dds(path.c_str()));
        if (tex.empty()) {
            DD_ERR("TextureLoader: could not load DDS '{}'", path);
            return result;
        }

        switch (tex.format()) {
        case gli::FORMAT_RGB_DXT1_UNORM_BLOCK8:
        case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
            result.props.m_Format = TextureFormat::FORMAT_BC1_RGB_UNORM;  break;
        case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
            result.props.m_Format = TextureFormat::FORMAT_BC3_RGBA_UNORM; break;
        case gli::FORMAT_RG_ATI2N_UNORM_BLOCK16:
            result.props.m_Format = TextureFormat::FORMAT_BC5_RG_UNORM;   break;
        case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
            result.props.m_Format = TextureFormat::FORMAT_BC7_RGBA_UNORM; break;
        default:
            DD_ERR("TextureLoader: unsupported DDS format {} in '{}'",
                   static_cast<int>(tex.format()), path);
            return result;
        }

        auto ext0 = tex.extent(0);
        result.props.m_Width      = static_cast<uint>(ext0.x);
        result.props.m_Height     = static_cast<uint>(ext0.y);
        result.props.m_MipmapMode = MipmapMode::Preloaded;
        result.props.m_MipLevels  = static_cast<uint32_t>(tex.levels());

        size_t totalBytes = 0;
        for (size_t lvl = 0; lvl < tex.levels(); ++lvl)
            totalBytes += tex.size(lvl);

        result.pixels.resize(totalBytes);
        result.mipOffsets.resize(tex.levels());

        size_t offset = 0;
        for (size_t lvl = 0; lvl < tex.levels(); ++lvl) {
            result.mipOffsets[lvl] = offset;
            size_t mipBytes = tex.size(lvl);
            std::memcpy(result.pixels.data() + offset, tex.data(0, 0, lvl), mipBytes);
            offset += mipBytes;
        }

        DD_INFO("TextureLoader: loaded DDS '{}' ({} mips)", path, tex.levels());
        return result;
    }

    int TextureLoader::GetDesiredChannels(const std::string& path)
    {
        int probeW, probeH, probeChannels;
        stbi_info(path.c_str(), &probeW, &probeH, &probeChannels);
        int desired = (probeChannels == 3 || probeChannels == 4) ? STBI_rgb_alpha : probeChannels;
        return desired;
    }

    TextureData TextureLoader::LoadHDR(const std::string& path)
    {
        TextureData result;
        int width, height, channels;
        int desiredChannels = GetDesiredChannels(path);

        stbi_set_flip_vertically_on_load(true);
        float* data = stbi_loadf(path.c_str(), &width, &height, &channels, desiredChannels);
        if (!data) {
            DD_ERR("TextureLoader: could not load HDR '{}'", path);
            return result;
        }

        result.props.m_Width = (uint)width;
        result.props.m_Height = (uint)height;
        
        // The typical industry standard is half float precision.
        switch (desiredChannels) {
        case 4:
            result.props.m_Format = TextureFormat::FORMAT_RGBA16F;
            break;
        default:
            DD_ERR("TextureLoader: Unsupported HDR channel count {} in '{}'", desiredChannels, path);
            stbi_image_free(data);
            return result;
        }

        const size_t byteCount = (size_t)width * height * desiredChannels * 2;
        result.pixels.resize(byteCount);

        const size_t pixels = width * height;
        // TODO:
        // Should be possible to optimize this with SIMD
        // Can also precompute the Alpha channel if it was padded which is usually the case
        for (int i = 0; i < pixels; ++i) {
            for (int j = 0; j < desiredChannels; ++j) {
                uint16_t half = Math::FloatToHalf(data[i * desiredChannels + j]);
                result.pixels[(i * desiredChannels + j) * 2] = half & 0xFF;
                result.pixels[(i * desiredChannels + j) * 2 + 1] = (half >> 8) & 0xFF;
            }
        }

        stbi_image_free(data);

        DD_INFO("TextureLoader: finished loading HDR texture to CPU '{}'", path);
        return result;
    }

    TextureData TextureLoader::LoadLDR(const std::string& path)
    {
        TextureData result;
        int width, height, channels;
        int desiredChannels = GetDesiredChannels(path);
        stbi_set_flip_vertically_on_load(true);
        uchar* data = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
        if (!data) {
            DD_ERR("TextureLoader: could not load '{}'", path);
            return result;
        }

        result.props.m_Width = (uint)width;
        result.props.m_Height = (uint)height;

        switch (desiredChannels) {
        case 1:
            result.props.m_Format = TextureFormat::FORMAT_RED;
            break;
        case 4:
            result.props.m_Format = TextureFormat::FORMAT_RGBA;
            break;
        default:
            DD_ERR("TextureLoader: unsupported channel count {} in '{}'", desiredChannels, path);
            stbi_image_free(data);
            return result;
        }

        result.pixels.assign(data, data + (size_t)width * height * desiredChannels);
        stbi_image_free(data);

        DD_INFO("TextureLoader: finished loading texture to CPU '{}'", path);
        return result;
    }

} // namespace Dodo
