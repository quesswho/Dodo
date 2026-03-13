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
              m_WrapV(SamplerWrapMode::WRAP_REPEAT)
        {}

        SamplerProperties(SamplerWrapMode wrap)
            : m_Filter(SamplerFilter::MIN_MAG_MIP_LINEAR), m_WrapU(wrap), m_WrapV(wrap)
        {}

        SamplerProperties(SamplerWrapMode wrapU, SamplerWrapMode wrapV)
            : m_Filter(SamplerFilter::MIN_MAG_MIP_LINEAR), m_WrapU(wrapU), m_WrapV(wrapV)
        {}

        SamplerProperties(SamplerFilter filter)
            : m_Filter(filter), m_WrapU(SamplerWrapMode::WRAP_REPEAT), m_WrapV(SamplerWrapMode::WRAP_REPEAT)
        {}

        SamplerProperties(SamplerFilter filter, SamplerWrapMode wrapU, SamplerWrapMode wrapV)
            : m_Filter(filter), m_WrapU(wrapU), m_WrapV(wrapV)
        {}

        SamplerFilter m_Filter;
        SamplerWrapMode m_WrapU;
        SamplerWrapMode m_WrapV;
    };
} // namespace Dodo
