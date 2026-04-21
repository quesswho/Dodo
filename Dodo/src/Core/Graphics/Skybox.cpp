#include "Skybox.h"
#include "pch.h"

namespace Dodo {

    static const float s_SkyboxVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                                             1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                                             -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                                             -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                                             1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                                             1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                                             -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                                             1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                                             -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                                             1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                                             -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                                             1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    Skybox::Skybox(std::vector<std::string> paths, AssetManager& assets, RenderAPI& renderAPI)
        : m_VertexBuffer(renderAPI.CreateVertexBuffer(s_SkyboxVertices, sizeof(s_SkyboxVertices),
                                                      BufferProperties({{"POSITION", 3}}))),
          m_Sampler(renderAPI.CreateSampler(SamplerProperties(SamplerFilter::MIN_MAG_MIP_LINEAR,
                                                              SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
                                                              SamplerWrapMode::WRAP_CLAMP_TO_EDGE))),
          m_Assets(assets)
    {
        m_CubeMapID = assets.LoadCubeMap(paths);

        ShaderID id = assets.LoadShaderFromPath("res/shader/builtin/Passes/Skybox.slang");
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = id;
        pipelineDesc.depthMode = DepthMode::LessEqual;
        pipelineDesc.culling = CullMode::None;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        m_Shader = assets.GetPipeline(pipelineID);
    }

    Skybox::~Skybox() {}

    void Skybox::Draw(RenderAPI& renderAPI) const
    {
        Ref<CubeMap> cubeMap = m_Assets.GetCubeMap(m_CubeMapID);
        if (!cubeMap) return; // Still loading

        renderAPI.BindPipeline(m_Shader);
        renderAPI.SetDrawData({.model = Math::Mat4(1.0f), .normalMatrix = Math::Mat3(1.0f)});
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindCubeMap(0, cubeMap);
        renderAPI.BindVertexBuffer(m_VertexBuffer);
        renderAPI.DrawArray(36);
    }
} // namespace Dodo
