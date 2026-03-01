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

    Skybox::Skybox(const Math::Mat4 &projection, std::vector<std::string> paths)
        : m_Projection(projection),
          m_Shader(Application::s_Application->m_AssetManager->GetShader(
              ShaderBuilderFlags::ShaderBuilderFlagCubeMap | ShaderBuilderFlags::ShaderBuilderFlagMaxDepth |
              ShaderBuilderFlags::ShaderBuilderFlagNoTexcoord)),
          m_VertexBuffer(
              new VertexBuffer(s_SkyboxVertices, sizeof(s_SkyboxVertices), BufferProperties({{"POSITION", 3}}))),
          m_CubeMapTexture(std::make_shared<CubeMapTexture>(paths, 0,
                                                            TextureSettings(TextureFilter::FILTER_MIN_MAG_MIP_LINEAR,
                                                                            TextureWrapMode::WRAP_CLAMP_TO_EDGE,
                                                                            TextureWrapMode::WRAP_CLAMP_TO_EDGE)))
    {}

    Skybox::~Skybox()
    {
        delete m_VertexBuffer;
    }

    void Skybox::Draw(const Math::Mat4 &viewMatrix) const
    {
        Application::s_Application->m_RenderAPI->DepthComparisonFunction(DepthComparisonFunction::LESS_EQUAL);
        m_Shader->Bind();
        m_Shader->SetUniformValue("u_Camera", m_Projection * Math::Mat4::RelinquishToMat3(viewMatrix));
        m_Shader->SetUniformValue("u_CubeMap", 0);
        m_CubeMapTexture->Bind();
        m_VertexBuffer->Bind();
        Application::s_Application->m_RenderAPI->DrawArray(36);
        Application::s_Application->m_RenderAPI->DepthComparisonFunction(DepthComparisonFunction::LESS);
    }
} // namespace Dodo