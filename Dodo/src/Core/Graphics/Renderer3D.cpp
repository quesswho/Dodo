#include "Renderer3D.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/Pipeline/ShaderCompiler.h"
#include "Core/Graphics/Pipeline/ShaderParser.h"

namespace Dodo {

    static std::string s_ShadowShader = R"(#shader fragment

	#version 420 core

	void main()
	{
		gl_FragDepth = gl_FragCoord.z;
	}

	#shader vertex

	#version 420 core
	layout(location = 0) in vec3 a_Position;

	layout(std140, binding = 0) uniform FrameData {
        mat4 u_Camera;
        mat4 u_LightCamera;
        vec3 u_LightDir;
        vec3 u_CameraPos;
    } frame;

	uniform mat4 u_Model;

	void main()
	{
		gl_Position = frame.u_LightCamera * u_Model * vec4(a_Position, 1.0);
	})";

    Renderer3D::Renderer3D(RenderAPI& renderAPI, AssetManager& assets) : m_ShadowMap(new ShadowMap())
    {
        ShaderID id = assets.LoadShader(ShaderParser::Parse(s_ShadowShader));
        PipelineDesc shadowPipelineDesc;
        shadowPipelineDesc.shaderID = id;
        shadowPipelineDesc.culling = true;
        shadowPipelineDesc.backfaceCull = false;
        PipelineID shadowPipelineID = assets.CreatePipeline(shadowPipelineDesc, renderAPI);
        m_ShadowMapMaterial = std::make_shared<Material>(assets.GetPipeline(shadowPipelineID));
    }

    void Renderer3D::RenderEntities(World& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
    {
        FrameData frameData;
        frameData.camera = camera.GetCameraMatrix();
        frameData.cameraPos = camera.GetPosition();
        frameData.lightCamera = lightSystem.m_Directional.m_LightCamera;
        frameData.lightDir = lightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(frameData); // Uploads frame data to the GPU

        // Draw ModelComponent
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            Model* model = assets.GetModel(modelComponent.m_ModelID);
            for (auto mesh : model->GetMeshes()) {
                Ref<Material> mat = mesh->GetMaterial();
                mat->Bind(renderAPI);
                renderAPI.SetDrawData({modelComponent.m_Transformation.m_Model});
                mesh->DrawGeometry(renderAPI);
            }
        }
    }

    void Renderer3D::RenderGeometry(World& world, RenderAPI& renderAPI, AssetManager& assets)
    {
        // Draw ModelComponents with custom material
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            renderAPI.SetDrawData({modelComponent.m_Transformation.m_Model});
            Model* model = assets.GetModel(modelComponent.m_ModelID);
            model->DrawGeometry(renderAPI);
        }
    }

    void Renderer3D::DrawScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI, AssetManager& assets)
    {
        RenderEntities(scene->GetWorld(), camera, scene->m_LightSystem, renderAPI, assets);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(camera.GetViewMatrix(), renderAPI);
    }

    void Renderer3D::DrawShadowedScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderAPI,
                                       AssetManager& assets)
    {
        // Draw to shadowmap
        FrameData frameData;
        frameData.camera = camera.GetCameraMatrix();
        frameData.cameraPos = camera.GetPosition();
        frameData.lightCamera = scene->m_LightSystem.m_Directional.m_LightCamera;
        frameData.lightDir = scene->m_LightSystem.m_Directional.m_Direction;
        renderAPI.SetFrameData(frameData);

        // Bind target, shadow pipeline and draw geometry to shadowmap
        m_ShadowMap->Bind();
        renderAPI.BindPipeline(m_ShadowMapMaterial->GetShader());
        World& world = scene->GetWorld();
        RenderGeometry(world, renderAPI, assets);

        // Bind postfx render target
        m_PostEffect->Bind();

        // Bind shadowmap to index 3
        renderAPI.BindFrameBufferTexture(3, m_ShadowMap->GetFrameBuffer());
        DrawScene(scene, camera, renderAPI, assets);

        // Draw postfx to screen target
        m_PostEffect->Draw(renderAPI);
    }
} // namespace Dodo