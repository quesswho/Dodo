#pragma once

#include "Material/SamplerProperties.h"

namespace Dodo {
    enum class FrameBufferType {
        FRAMEBUFFER_COLOR_DEPTH_STENCIL,
        FRAMEBUFFER_DEPTH
    };

    struct FrameBufferProperties {
        FrameBufferProperties()
            : m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL),
              m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type)
            : m_Width(width), m_Height(height), m_FrameBufferType(type),
              m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        uint m_Width, m_Height;
        FrameBufferType m_FrameBufferType;
        SamplerProperties m_SamplerProperties;
    };
} // namespace Dodo