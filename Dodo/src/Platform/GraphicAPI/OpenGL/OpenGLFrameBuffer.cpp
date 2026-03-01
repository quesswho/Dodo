#include "OpenGLFrameBuffer.h"
#include "pch.h"

namespace Dodo { namespace Platform {

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferProperties &framebufferProp)
        : m_FrameBufferProperties(framebufferProp)
    {
        Create();
    }

    void OpenGLFrameBuffer::Create()
    {
        glGenFramebuffers(1, &m_FrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        if (m_FrameBufferProperties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_FrameBufferProperties.m_Width, m_FrameBufferProperties.m_Height,
                         0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

            glGenRenderbuffers(1, &m_RenderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_FrameBufferProperties.m_Width,
                                  m_FrameBufferProperties.m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else if (m_FrameBufferProperties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_DEPTH)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_FrameBufferProperties.m_Width,
                         m_FrameBufferProperties.m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_TextureID, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
}} // namespace Dodo::Platform