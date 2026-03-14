#include "OpenGLSampler.h"

#include <glad/gl.h>

namespace Dodo::Platform {

    OpenGLSampler::OpenGLSampler(const SamplerProperties& prop)
    {
        glCreateSamplers(1, &m_SamplerID);

        // TODO: Add to sampler properties
        static const float borderColor[] = {1.0f, 0.4f, 0.8f, 0.09f};

        auto ApplyWrap = [&](GLenum axis, SamplerWrapMode mode) {
            switch (mode) {
            case SamplerWrapMode::WRAP_REPEAT:
                glSamplerParameteri(m_SamplerID, axis, GL_REPEAT);
                break;
            case SamplerWrapMode::WRAP_CLAMP_TO_BORDER:
                glSamplerParameteri(m_SamplerID, axis, GL_CLAMP_TO_BORDER);
                glSamplerParameterfv(m_SamplerID, GL_TEXTURE_BORDER_COLOR, prop.m_BorderColor);
                break;
            case SamplerWrapMode::WRAP_CLAMP_TO_EDGE:
                glSamplerParameteri(m_SamplerID, axis, GL_CLAMP_TO_EDGE);
                break;
            case SamplerWrapMode::WRAP_MIRRORED_REPEAT:
                glSamplerParameteri(m_SamplerID, axis, GL_MIRRORED_REPEAT);
                break;
            }
        };

        ApplyWrap(GL_TEXTURE_WRAP_S, prop.m_WrapU);
        ApplyWrap(GL_TEXTURE_WRAP_T, prop.m_WrapV);

        switch (prop.m_Filter) {
        case SamplerFilter::MIN_MAG_MIP_LINEAR:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::MIN_LINEAR_MAG_MIP_NEAREST:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_MAG_LINEAR_MIP_NEAREST:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::FILTER_MIN_MAG_MIP_NEAREST:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::FILTER_MIN_MAG_NEAREST_MIP_LINEAR:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::FILTER_MIN_NEAREST_MAG_MIP_LINEAR:
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
    }

    OpenGLSampler::~OpenGLSampler()
    {
        glDeleteSamplers(1, &m_SamplerID);
    }
} // namespace Dodo::Platform