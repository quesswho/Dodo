#include "Skybox.h"
#include "pch.h"

#include "Core/Application/Application.h"

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

    Skybox::Skybox(const Math::Mat4& projection, std::vector<std::string> paths, AssetManager& assets)
        : m_Projection(projection),
          m_VertexBuffer(std::make_unique<VertexBuffer>(s_SkyboxVertices, sizeof(s_SkyboxVertices),
                                                        BufferProperties({{"POSITION", 3}}))),
          m_Sampler(std::make_shared<TextureSampler>(SamplerProperties(SamplerFilter::MIN_MAG_MIP_LINEAR,
                                                                       SamplerWrapMode::WRAP_CLAMP_TO_EDGE,
                                                                       SamplerWrapMode::WRAP_CLAMP_TO_EDGE))),
          m_CubeMap(std::make_shared<CubeMap>(paths))
    {
        ShaderID id = assets.LoadShader(ShaderBuilderFlags::ShaderBuilderFlagCubeMap |
                                        ShaderBuilderFlags::ShaderBuilderFlagMaxDepth |
                                        ShaderBuilderFlags::ShaderBuilderFlagNoTexcoord);
        m_Shader = assets.GetShader(id);
    }

    Skybox::~Skybox() {}

    void Skybox::Draw(const Math::Mat4& viewMatrix, RenderAPI& renderAPI) const
    {
        renderAPI.DepthComparisonFunction(DepthComparisonFunction::LESS_EQUAL);
        m_Shader->Bind();
        m_Shader->SetUniformValue("u_Camera", m_Projection * Math::Mat4::RelinquishToMat3(viewMatrix));
        m_Shader->SetUniformValue("u_CubeMap", 0);
        renderAPI.BindTextureSampler(0, m_Sampler);
        renderAPI.BindCubeMap(0, m_CubeMap);
        m_VertexBuffer->Bind();
        renderAPI.DrawArray(36);
        renderAPI.DepthComparisonFunction(DepthComparisonFunction::LESS);
    }
} // namespace Dodo