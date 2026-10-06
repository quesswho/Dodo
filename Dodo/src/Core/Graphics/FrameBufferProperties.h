#pragma once

#include "Material/SamplerProperties.h"

namespace Dodo {
    enum class FrameBufferColorFormat {
        RGBA16F,
        RGBA8
    };

    enum class FrameBufferType {
        FRAMEBUFFER_COLOR_DEPTH_STENCIL,
        FRAMEBUFFER_DEPTH,
        FRAMEBUFFER_DEPTH_ARRAY
    };

    struct FrameBufferProperties {
        FrameBufferProperties()
            : m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL), m_Layers(1),
              m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type)
            : m_Width(width), m_Height(height), m_FrameBufferType(type), m_Layers(1),
              m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type, uint32_t layers)
            : m_Width(width), m_Height(height), m_FrameBufferType(type), m_Layers(layers),
              m_SamplerProperties(SamplerProperties(SamplerFilter::MIN_MAG_LINEAR))
        {}

        uint m_Width, m_Height;
        FrameBufferType m_FrameBufferType;
        uint32_t m_Layers;
        SamplerProperties m_SamplerProperties;
        FrameBufferColorFormat m_ColorFormat = FrameBufferColorFormat::RGBA16F;

        /**
         * MSAA sample count of a color framebuffer, a power of two. Values above 1 render into multisampled
         * attachments that are resolved into the sampled color image at the end of the pass. Clamped to what
         * the device supports. Depth-only framebuffers are always single sampled.
         */
        uint32_t m_Samples = 1;
    };
} // namespace Dodo