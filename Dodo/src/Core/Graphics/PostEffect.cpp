#include "PostEffect.h"
#include "pch.h"

namespace Dodo {
    PostEffect::PostEffect(const FrameBufferProperties &framebufferprop, const char *path)
        : m_Framebuffer(new FrameBuffer(framebufferprop)), m_Shader(Shader::CreateFromPath("Postfx", path))
    {
        Create();
    }

    void PostEffect::Create()
    {
        float screenQuad[] = {
            -1.0f, 1.0f,  0.0f, 1.0f, // top left
            -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom right

            -1.0f, 1.0f,  0.0f, 1.0f, // top left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom right
            1.0f,  1.0f,  1.0f, 1.0f  // top right
        };
        m_Vertexbuffer =
            new VertexBuffer(screenQuad, 6 * 4 * sizeof(float), BufferProperties({{"POSITION", 2}, {"TEXCOORD", 2}}));
    }

    PostEffect::~PostEffect()
    {
        delete m_Framebuffer;
        delete m_Vertexbuffer;
    }
} // namespace Dodo