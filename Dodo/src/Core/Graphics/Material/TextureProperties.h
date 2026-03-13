#pragma once

#include <Core/Common.h>

namespace Dodo {

    enum class TextureFormat : uint8_t {
        FORMAT_RED,
        FORMAT_RGB,
        FORMAT_RGBA
    };

    struct TextureProperties {
        TextureProperties() : m_Width(0), m_Height(0), m_Format(TextureFormat::FORMAT_RGB) {}

        TextureProperties(uint width, uint height)
            : m_Width(width), m_Height(height), m_Format(TextureFormat::FORMAT_RGB)
        {}

        TextureProperties(uint width, uint height, TextureFormat format)
            : m_Width(width), m_Height(height), m_Format(format)
        {}

        uint m_Width;
        uint m_Height;
        TextureFormat m_Format;
    };
} // namespace Dodo
