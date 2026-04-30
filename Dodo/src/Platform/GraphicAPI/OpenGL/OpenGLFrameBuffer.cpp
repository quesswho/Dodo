#include "OpenGLFrameBuffer.h"

#include "Core/Utilities/Logger.h"

namespace Dodo::Platform {
    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferProperties& framebufferProp)
        : m_FrameBufferProperties(framebufferProp), m_Layers(framebufferProp.m_Layers)
    {
        Create();
    }

    void OpenGLFrameBuffer::Create()
    {
        glGenFramebuffers(1, &m_FrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        if (m_FrameBufferProperties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_FrameBufferProperties.m_Width, m_FrameBufferProperties.m_Height,
                         0, GL_RGB, GL_FLOAT, 0);
            SetFilter(m_FrameBufferProperties.m_SamplerProperties.m_Filter);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

            glGenRenderbuffers(1, &m_RenderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_FrameBufferProperties.m_Width,
                                  m_FrameBufferProperties.m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
        } else if (m_FrameBufferProperties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_DEPTH) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_FrameBufferProperties.m_Width,
                         m_FrameBufferProperties.m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            SetFilter(m_FrameBufferProperties.m_SamplerProperties.m_Filter);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_TextureID, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        } else if (m_FrameBufferProperties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_DEPTH_ARRAY) {
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_TextureID);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, m_FrameBufferProperties.m_Width,
                         m_FrameBufferProperties.m_Height, m_Layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            constexpr float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
            // Layered attachment: geometry shader selects the layer via gl_Layer
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_TextureID, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        m_Sampler = std::make_shared<OpenGLSampler>(m_FrameBufferProperties.m_SamplerProperties);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            DD_ERR("Framebuffer incomplete: {}", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
     * Unfortunately OpenGLSampler is not compatible with ImGui as far as I know. Therefore we must set the sampler
     * filters directly in to the textures.
     */
    void OpenGLFrameBuffer::SetFilter(SamplerFilter filter)
    {
        switch (filter) {
        case SamplerFilter::MIN_MAG_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_MAG_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::MIN_MAG_MIP_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::MIN_LINEAR_MAG_MIP_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_MAG_LINEAR_MIP_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::MIN_MAG_MIP_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_MAG_NEAREST_MIP_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerFilter::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilter::MIN_NEAREST_MAG_MIP_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteTextures(1, &m_TextureID);
        glDeleteRenderbuffers(1, &m_RenderBuffer);
        glDeleteFramebuffers(1, &m_FrameBufferID);
    }

    void OpenGLFrameBuffer::Resize(uint width, uint height)
    {
        m_FrameBufferProperties.m_Width = width;
        m_FrameBufferProperties.m_Height = height;

        glDeleteTextures(1, &m_TextureID);
        glDeleteRenderbuffers(1, &m_RenderBuffer);
        glDeleteFramebuffers(1, &m_FrameBufferID);
        Create();
    }
} // namespace Dodo::Platform