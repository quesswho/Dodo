#include "PostEffect.h"
#include "Core/Application/Application.h"
#include "pch.h"

namespace Dodo {
    PostEffect::PostEffect(const FrameBufferProperties& framebufferprop, const char* path, RenderAPI& renderAPI,
                           AssetManager& assets)
        : m_Framebuffer(new FrameBuffer(framebufferprop))
    {
        ShaderID id = assets.LoadShaderFromPath(path);
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = id;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        m_Shader = assets.GetPipeline(pipelineID);
        Create();
    }

    void PostEffect::Draw(RenderAPI& renderAPI) const
    {
        renderAPI.DefaultFrameBuffer();
        renderAPI.BindPipeline(m_Shader);
        m_Vertexbuffer->Bind();
        m_Framebuffer->BindTexture();
        renderAPI.DrawArray(6);
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