#include "TextureLoader.h"

#include "Core/Math/MathFunc.h"
#include "Core/Utilities/Logger.h"

#include <stb_image.h>
#include <gli/gli.hpp>

#include <cctype>
#include <cstring>
#include <filesystem>

namespace Dodo {

    TextureData TextureLoader::Load(const std::string& path)
    {
        std::string ext = std::filesystem::path(path).extension().string();
        for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
        if (ext == ".dds")
            return LoadDDS(path);
        return stbi_is_hdr(path.c_str()) ? LoadHDR(path) : LoadLDR(path);
    }

    TextureData TextureLoader::LoadDDS(const std::string& path)
    {
        TextureData result;

        gli::texture2d tex(gli::texture2d(gli::load_dds(path.c_str())));
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
            auto mipExt = tex.extent((int)lvl);
            FlipMipDDS(result.pixels.data() + offset,
                       (uint32_t)mipExt.x, (uint32_t)mipExt.y,
                       result.props.m_Format);
            offset += mipBytes;
        }

        DD_INFO("TextureLoader: loaded DDS '{}' ({} mips)", path, tex.levels());
        return result;
    }

    void TextureLoader::FlipBC1Block(uint8_t* block)
    {
        // bytes 0-3: two RGB565 color endpoints, unchanged
        // bytes 4-7: one byte per pixel-row (4 x 2-bit indices per row)
        std::swap(block[4], block[7]);
        std::swap(block[5], block[6]);
    }

    void TextureLoader::FlipBC4Block(uint8_t* block)
    {
        // bytes 0-1: endpoints, unchanged
        // bytes 2-7: 48-bit field of 16 x 3-bit indices, 4 indices (12 bits) per row
        uint64_t bits = 0;
        for (int i = 0; i < 6; ++i)
            bits |= (uint64_t)block[2 + i] << (i * 8);

        const uint64_t row0 = (bits >>  0) & 0xFFF;
        const uint64_t row1 = (bits >> 12) & 0xFFF;
        const uint64_t row2 = (bits >> 24) & 0xFFF;
        const uint64_t row3 = (bits >> 36) & 0xFFF;

        bits = row3 | (row2 << 12) | (row1 << 24) | (row0 << 36);
        for (int i = 0; i < 6; ++i)
            block[2 + i] = (uint8_t)((bits >> (i * 8)) & 0xFF);
    }

    void TextureLoader::FlipMipDDS(uint8_t* data, uint32_t width, uint32_t height, TextureFormat fmt)
    {
        const uint32_t blockW     = (width  + 3) / 4;
        const uint32_t blockH     = (height + 3) / 4;
        const size_t   blockBytes = (fmt == TextureFormat::FORMAT_BC1_RGB_UNORM) ? 8u : 16u;
        const size_t   rowStride  = blockW * blockBytes;

        for (uint32_t r = 0; r < blockH / 2; ++r) {
            uint8_t* rowA = data + r                * rowStride;
            uint8_t* rowB = data + (blockH - 1 - r) * rowStride;
            for (size_t i = 0; i < rowStride; ++i)
                std::swap(rowA[i], rowB[i]);
        }

        for (uint32_t r = 0; r < blockH; ++r) {
            uint8_t* row = data + r * rowStride;
            for (uint32_t c = 0; c < blockW; ++c) {
                uint8_t* blk = row + c * blockBytes;
                switch (fmt) {
                case TextureFormat::FORMAT_BC1_RGB_UNORM:
                    FlipBC1Block(blk);
                    break;
                case TextureFormat::FORMAT_BC3_RGBA_UNORM:
                    FlipBC4Block(blk);      // alpha sub-block (bytes 0-7)
                    FlipBC1Block(blk + 8);  // color sub-block (bytes 8-15)
                    break;
                case TextureFormat::FORMAT_BC5_RG_UNORM:
                    FlipBC4Block(blk);      // red channel (bytes 0-7)
                    FlipBC4Block(blk + 8);  // green channel (bytes 8-15)
                    break;
                case TextureFormat::FORMAT_BC7_RGBA_UNORM:
                    // BC7 has 8 variable-bit modes; within-block row flip is not implemented.
                    // The block-row swap above corrects gross orientation.
                    break;
                default: break;
                }
            }
        }
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
