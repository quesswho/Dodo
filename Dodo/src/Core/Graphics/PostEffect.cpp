#include "PostEffect.h"
#include "Core/Application/Application.h"

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

        m_Vertexbuffer = assets.GetScreenQuadBuffer(renderAPI);
    }

    void PostEffect::Draw(RenderAPI& renderAPI) const
    {
        renderAPI.DefaultFrameBuffer();
        renderAPI.BindPipeline(m_Shader);
        renderAPI.PushConstants(m_PushConstantData.data(), m_PushConstantData.size());
        renderAPI.BindVertexBuffer(m_Vertexbuffer);
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindFrameBufferTexture(0, m_Framebuffer);
        renderAPI.SetDrawData({.model = Math::Mat4(1.0f), .normalMatrix = Math::Mat3(1.0f)});
        renderAPI.DrawArray(6);
    }

    PostEffect::~PostEffect() {}
} // namespace Dodo