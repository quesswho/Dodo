#pragma once

#include <Core/Common.h>

namespace Dodo {

    enum class TextureFormat : uint8_t {
        FORMAT_RED,
        FORMAT_RGB,
        FORMAT_RGBA,
        FORMAT_RGB16F,  // 16-bit float RGB, half-precision HDR (not currently produced by loader)
        FORMAT_RGBA16F, // 16-bit float RGB, half-precision HDR
        FORMAT_RGB32F,  // 32-bit float RGB, full-precision HDR (not currently produced by loader)
        FORMAT_RGBA32F  // 32-bit float RGBA, full-precision HDR (not currently produced by loader)
    };

    struct TextureProperties {
        TextureProperties() : m_Width(0), m_Height(0), m_Format(TextureFormat::FORMAT_RGBA) {}

        TextureProperties(uint width, uint height)
            : m_Width(width), m_Height(height), m_Format(TextureFormat::FORMAT_RGBA)
        {}

        TextureProperties(uint width, uint height, TextureFormat format)
            : m_Width(width), m_Height(height), m_Format(format)
        {}

        uint m_Width;
        uint m_Height;
        TextureFormat m_Format;
    };
} // namespace Dodo
