#include "Renderer3D.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/Shader/ShaderCompiler.h"
#include "Core/Graphics/Shader/ShaderParser.h"

namespace Dodo {

    static std::string s_ShadowShader = R"(#shader fragment

	#version 330 core

	void main()
	{
		gl_FragDepth = gl_FragCoord.z;
	}

	#shader vertex

	#version 330 core
	layout(location = 0) in vec3 a_Position;

	uniform mat4 u_LightCamera;
	uniform mat4 u_Model;

	void main()
	{
		gl_Position = u_LightCamera * u_Model * vec4(a_Position, 1.0);
	})";

    Renderer3D::Renderer3D(Math::FreeCamera* camera) : m_Camera(camera), m_ShadowMap(new ShadowMap())
    {
        ShaderID id = Application::s_Application->m_AssetManager->LoadShader(ShaderParser::Parse(s_ShadowShader));
        m_ShadowMapMaterial = std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(id));
    }

    void Renderer3D::RenderEntities(World& world, Math::FreeCamera* camera, LightSystem& lightSystem,
                                    RenderAPI& renderAPI, AssetManager& assets)
    {
        // Draw ModelComponent
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            Model* model = assets.GetModel(modelComponent.m_ModelID);
            for (auto mesh : model->GetMeshes()) {
                Ref<Material> mat = mesh->GetMaterial();
                mat->Bind(renderAPI);
                mat->SetUniform("u_LightCamera", lightSystem.m_Directional.m_LightCamera);
                mat->SetUniform("u_LightDir", lightSystem.m_Directional.m_Direction);
                mat->SetUniform("u_Model", modelComponent.m_Transformation.m_Model);
                mat->SetUniform("u_Camera", camera->GetCameraMatrix());
                mat->SetUniform("u_CameraPos", camera->GetCameraPos());
                mesh->DrawGeometry(renderAPI);
            }
        }
    }

    void Renderer3D::RenderEntitiesWithMaterial(World& world, Ref<Material> material, RenderAPI& renderAPI,
                                                AssetManager& assets)
    {
        material->Bind(renderAPI);

        // Draw ModelComponents with custom material
        const auto& modelPool = world.GetPool<ModelComponent>();
        for (const auto& modelComponent : modelPool.GetComponents()) {
            material->SetUniform("u_Model", modelComponent.m_Transformation.m_Model);
            Model* model = assets.GetModel(modelComponent.m_ModelID);
            model->DrawGeometry(renderAPI);
        }
    }

    void Renderer3D::DrawScene(Scene* scene, RenderAPI& renderAPI, AssetManager& assets)
    {
        RenderEntities(scene->GetWorld(), m_Camera, scene->m_LightSystem, renderAPI, assets);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix(), renderAPI);
    }

    void Renderer3D::DrawShadowedScene(Scene* scene, RenderAPI& renderAPI, AssetManager& assets)
    {

        // Bind shadowmap
        m_ShadowMap->Bind();

        // Draw to shadowmap
        renderAPI.Culling(true, false);
        m_ShadowMapMaterial->GetShader()->Bind();
        m_ShadowMapMaterial->SetUniform("u_LightCamera", scene->m_LightSystem.m_Directional.m_LightCamera);
        World& world = scene->GetWorld();
        RenderEntitiesWithMaterial(world, m_ShadowMapMaterial, renderAPI, assets);
        renderAPI.Culling(renderAPI.m_CullingDefault, true);

        // Bind postfx render target
        m_PostEffect->Bind();

        // Bind shadowmap to index 3
        m_ShadowMap->BindTexture(3);
        DrawScene(scene, renderAPI, assets);

        // Draw postfx to screen target
        m_PostEffect->Draw();
    }
} // namespace Dodo