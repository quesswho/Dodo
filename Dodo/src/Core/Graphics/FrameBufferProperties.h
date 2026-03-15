#pragma once

namespace Dodo {
    enum class FrameBufferType {
        FRAMEBUFFER_COLOR_DEPTH_STENCIL,
        FRAMEBUFFER_DEPTH
    };

    struct FrameBufferProperties {
        FrameBufferProperties()
            : m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL)
        {}

        FrameBufferProperties(uint width, uint height, FrameBufferType type)
            : m_Width(width), m_Height(height), m_FrameBufferType(type)
        {}

        uint m_Width, m_Height;
        FrameBufferType m_FrameBufferType;
    };
} // namespace Dodo