#pragma once

#include <Core/Common.h>

namespace Dodo {

    enum class TextureFormat : uint8_t
    {
        FORMAT_RED,
        FORMAT_RGB,
        FORMAT_RGBA
    };

    struct TextureProperties {
        TextureProperties() : m_Width(0), m_Height(0), m_Format(TextureFormat::FORMAT_RGB)
        {}

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

    enum class TextureFilter : uint32_t
    {
        FILTER_MIN_MAG_LINEAR_MIP_NEAREST,
        FILTER_MIN_MAG_MIP_LINEAR,
        FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
        FILTER_MIN_NEAREST_MAG_MIP_LINEAR,

        FILTER_MIN_MAG_MIP_NEAREST,
        FILTER_MIN_MAG_NEAREST_MIP_LINEAR,
        FILTER_MIN_LINEAR_MAG_MIP_NEAREST,
        FILTER_MIN_LINEAR_MAG_MIP_LINEAR
    };

    enum class TextureWrapMode
    {
        WRAP_REPEAT,
        WRAP_CLAMP_TO_BORDER,
        WRAP_CLAMP_TO_EDGE,
        WRAP_MIRRORED_REPEAT
    };

    struct TextureSettings {
        TextureSettings()
            : m_Filter(TextureFilter::FILTER_MIN_MAG_MIP_LINEAR), m_WrapU(TextureWrapMode::WRAP_REPEAT),
              m_WrapV(TextureWrapMode::WRAP_REPEAT)
        {}

        TextureSettings(TextureWrapMode wrap)
            : m_Filter(TextureFilter::FILTER_MIN_MAG_MIP_LINEAR), m_WrapU(wrap), m_WrapV(wrap)
        {}

        TextureSettings(TextureWrapMode wrapU, TextureWrapMode wrapV)
            : m_Filter(TextureFilter::FILTER_MIN_MAG_MIP_LINEAR), m_WrapU(wrapU), m_WrapV(wrapV)
        {}

        TextureSettings(TextureFilter filter)
            : m_Filter(filter), m_WrapU(TextureWrapMode::WRAP_REPEAT), m_WrapV(TextureWrapMode::WRAP_REPEAT)
        {}

        TextureSettings(TextureFilter filter, TextureWrapMode wrapU, TextureWrapMode wrapV)
            : m_Filter(filter), m_WrapU(wrapU), m_WrapV(wrapV)
        {}

        TextureFilter m_Filter;
        TextureWrapMode m_WrapU;
        TextureWrapMode m_WrapV;
    };
} // namespace Dodo