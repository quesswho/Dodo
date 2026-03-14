#include <Core/Common.h>

namespace Dodo {
    enum class SamplerFilter : uint32_t {
        MIN_MAG_MIP_LINEAR,
        MIN_LINEAR_MAG_MIP_NEAREST,
        MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
        MIN_MAG_LINEAR_MIP_NEAREST,
        FILTER_MIN_MAG_MIP_NEAREST,
        FILTER_MIN_MAG_NEAREST_MIP_LINEAR,
        FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
        FILTER_MIN_NEAREST_MAG_MIP_LINEAR,
    };

    enum class SamplerWrapMode {
        WRAP_REPEAT,
        WRAP_CLAMP_TO_BORDER,
        WRAP_CLAMP_TO_EDGE,
        WRAP_MIRRORED_REPEAT
    };

    struct SamplerProperties {
        SamplerProperties()
            : m_Filter(SamplerFilter::MIN_MAG_MIP_LINEAR), m_WrapU(SamplerWrapMode::WRAP_REPEAT),
              m_WrapV(SamplerWrapMode::WRAP_REPEAT), m_BorderColor{1.0f, 0.4f, 0.8f, 0.09f}
        {}
        SamplerProperties(SamplerWrapMode wrap)
            : m_Filter(SamplerFilter::MIN_MAG_MIP_LINEAR), m_WrapU(wrap), m_WrapV(wrap),
              m_BorderColor{1.0f, 0.4f, 0.8f, 0.09f}
        {}
        SamplerProperties(SamplerWrapMode wrapU, SamplerWrapMode wrapV)
            : m_Filter(SamplerFilter::MIN_MAG_MIP_LINEAR), m_WrapU(wrapU), m_WrapV(wrapV),
              m_BorderColor{1.0f, 0.4f, 0.8f, 0.09f}
        {}
        SamplerProperties(SamplerFilter filter)
            : m_Filter(filter), m_WrapU(SamplerWrapMode::WRAP_REPEAT), m_WrapV(SamplerWrapMode::WRAP_REPEAT),
              m_BorderColor{1.0f, 0.4f, 0.8f, 0.09f}
        {}
        SamplerProperties(SamplerFilter filter, SamplerWrapMode wrapU, SamplerWrapMode wrapV)
            : m_Filter(filter), m_WrapU(wrapU), m_WrapV(wrapV), m_BorderColor{1.0f, 0.4f, 0.8f, 0.09f}
        {}

        SamplerProperties& WithBorderColor(float r, float g, float b, float a)
        {
            m_BorderColor[0] = r;
            m_BorderColor[1] = g;
            m_BorderColor[2] = b;
            m_BorderColor[3] = a;
            return *this;
        }

        SamplerFilter m_Filter;
        SamplerWrapMode m_WrapU;
        SamplerWrapMode m_WrapV;
        float m_BorderColor[4];
    };
} // namespace Dodo
