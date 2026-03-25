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
        : m_VertexBuffer(std::make_unique<VertexBuffer>(s_SkyboxVertices, sizeof(s_SkyboxVertices),
                                                        BufferProperties({{"POSITION", 3}}))),
          m_Sampler(std::make_shared<TextureSampler>(SamplerProperties(SamplerFilter::MIN_MAG_MIP_LINEAR,
                                                                       SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
                                                                       SamplerWrapMode::WRAP_CLAMP_TO_EDGE))),
          m_CubeMap(std::make_shared<CubeMap>(paths))
    {
        ShaderID id = assets.LoadShader(ShaderBuilderFlags::ShaderBuilderFlagCubeMap |
                                            ShaderBuilderFlags::ShaderBuilderFlagMaxDepth |
                                            ShaderBuilderFlags::ShaderBuilderFlagNoTexcoord,
                                        renderAPI);
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = id;
        pipelineDesc.depthMode = DepthComparisonMethod::LESS_EQUAL;
        pipelineDesc.culling = false;
        pipelineDesc.backfaceCull = false;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        m_Shader = assets.GetPipeline(pipelineID);
    }

    Skybox::~Skybox() {}

    void Skybox::Draw(const Math::FreeCamera& camera, RenderAPI& renderAPI) const
    {
        renderAPI.BindPipeline(m_Shader);
        FrameData skyboxFrame{};
        skyboxFrame.camera = camera.GetProjectionMatrix() * Math::Mat4::RelinquishToMat3(camera.GetViewMatrix());
        renderAPI.SetFrameData(
            skyboxFrame); // Overrides UBO for skybox. Note a better way would be to have a separate UBO for skybox data
        renderAPI.SetDrawData({.model = Math::Mat4(1.0f), .normalMatrix = Math::Mat3(1.0f)});
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindCubeMap(0, m_CubeMap);
        m_VertexBuffer->Bind();
        renderAPI.DrawArray(36);
    }
} // namespace Dodo
