#pragma once

#include <Core/Common.h>

namespace Dodo {

    enum class MipmapMode : uint8_t { None, Generated, Preloaded };

    enum class TextureFormat : uint8_t {
        FORMAT_RED,
        FORMAT_RGB,
        FORMAT_RGBA,
        FORMAT_RGB16F,  // 16-bit float RGB, half-precision HDR (not currently produced by loader)
        FORMAT_RGBA16F, // 16-bit float RGB, half-precision HDR
        FORMAT_RGB32F,  // 32-bit float RGB, full-precision HDR (not currently produced by loader)
        FORMAT_RGBA32F, // 32-bit float RGBA, full-precision HDR (not currently produced by loader)
        FORMAT_BC1_RGB_UNORM,  // BC1/DXT1, 8 bytes/block, RGB (no alpha)
        FORMAT_BC3_RGBA_UNORM, // BC3/DXT5, 16 bytes/block, RGBA
        FORMAT_BC5_RG_UNORM,   // BC5/RGTC2, 16 bytes/block, RG (normal maps)
        FORMAT_BC7_RGBA_UNORM  // BC7/BPTC, 16 bytes/block, high-quality RGBA
    };

    struct TextureProperties {
        TextureProperties()
            : m_Width(0), m_Height(0), m_Format(TextureFormat::FORMAT_RGBA),
              m_MipmapMode(MipmapMode::Generated), m_MipLevels(0)
        {}

        TextureProperties(uint width, uint height)
            : m_Width(width), m_Height(height), m_Format(TextureFormat::FORMAT_RGBA),
              m_MipmapMode(MipmapMode::Generated), m_MipLevels(0)
        {}

        TextureProperties(uint width, uint height, TextureFormat format)
            : m_Width(width), m_Height(height), m_Format(format),
              m_MipmapMode(MipmapMode::Generated), m_MipLevels(0)
        {}

        uint m_Width;
        uint m_Height;
        TextureFormat m_Format;
        MipmapMode m_MipmapMode = MipmapMode::Generated;
        uint32_t m_MipLevels = 0; // only used when m_MipmapMode == Preloaded
    };
} // namespace Dodo
