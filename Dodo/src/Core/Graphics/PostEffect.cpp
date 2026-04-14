#include "PostEffect.h"
#include "Core/Application/Application.h"
#include "pch.h"

namespace Dodo {
    PostEffect::PostEffect(const FrameBufferProperties& framebufferprop, const char* path, RenderAPI& renderAPI,
                           AssetManager& assets)
        : m_Framebuffer(renderAPI.CreateFrameBuffer(framebufferprop))
    {
        ShaderID id = assets.LoadShaderFromPath(path);
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = id;
        pipelineDesc.depthMode = DepthMode::None;
        pipelineDesc.culling = CullMode::None;
        pipelineDesc.renderToSwapchain = true;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        m_Shader = assets.GetPipeline(pipelineID);

        m_Sampler = renderAPI.CreateSampler(SamplerProperties(
            SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_EDGE, SamplerWrapMode::WRAP_CLAMP_TO_EDGE));

        float screenQuad[] = {
            -1.0f, 1.0f,  0.0f, 1.0f, // top left
            -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom right

            -1.0f, 1.0f,  0.0f, 1.0f, // top left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom right
            1.0f,  1.0f,  1.0f, 1.0f  // top right
        };
        m_Vertexbuffer = renderAPI.CreateVertexBuffer(screenQuad, 6 * 4 * sizeof(float),
                                                      BufferProperties({{"POSITION", 2}, {"TEXCOORD", 2}}));
    }

    void PostEffect::Draw(RenderAPI& renderAPI) const
    {
        renderAPI.DefaultFrameBuffer();
        renderAPI.BindPipeline(m_Shader);
        renderAPI.PushConstants(m_PushConstantData.data(), m_PushConstantData.size());
        renderAPI.BindVertexBuffer(m_Vertexbuffer);
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindFrameBufferTexture(0, m_Framebuffer);
        renderAPI.DrawArray(6);
    }

    PostEffect::~PostEffect() {}
} // namespace Dodo