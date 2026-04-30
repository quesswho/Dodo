#pragma once

#include "Material/SamplerProperties.h"

namespace Dodo {
    enum class FrameBufferType {
        FRAMEBUFFER_COLOR_DEPTH_STENCIL,
        FRAMEBUFFER_DEPTH,
        FRAMEBUFFER_DEPTH_ARRAY
    };

    struct FrameBufferProperties {
        FrameBufferProperties()
            : m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL),
              m_Layers(1), m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type)
            : m_Width(width), m_Height(height), m_FrameBufferType(type),
              m_Layers(1), m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type, uint32_t layers)
            : m_Width(width), m_Height(height), m_FrameBufferType(type),
              m_Layers(layers), m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        uint m_Width, m_Height;
        FrameBufferType m_FrameBufferType;
        uint32_t m_Layers;
        SamplerProperties m_SamplerProperties;
    };
} // namespace Dodo