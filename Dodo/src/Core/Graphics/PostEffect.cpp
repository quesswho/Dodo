#include "PostEffect.h"
#include "Core/Application/Application.h"
#include "pch.h"

namespace Dodo {
    PostEffect::PostEffect(const FrameBufferProperties& framebufferprop, const char* path, RenderAPI& renderAPI,
                           AssetManager& assets)
        : m_Framebuffer(std::make_shared<FrameBuffer>(framebufferprop))
    {
        ShaderID id = assets.LoadShaderFromPath(path);
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = id;
        pipelineDesc.depthMode = DepthMode::None;
        pipelineDesc.culling = CullMode::None;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        m_Shader = assets.GetPipeline(pipelineID);

        m_Sampler = std::make_shared<TextureSampler>(SamplerProperties(
            SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_EDGE, SamplerWrapMode::WRAP_CLAMP_TO_EDGE));

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

    void PostEffect::Draw(RenderAPI& renderAPI) const
    {
        renderAPI.DefaultFrameBuffer();
        renderAPI.BindPipeline(m_Shader);
        renderAPI.PushConstants(m_PushConstantData.data(), m_PushConstantData.size());
        m_Vertexbuffer->Bind();
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindFrameBufferTexture(0, m_Framebuffer);
        renderAPI.DrawArray(6);
    }

    PostEffect::~PostEffect()
    {
        delete m_Vertexbuffer;
    }
} // namespace Dodo