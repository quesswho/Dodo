#include "Renderer3D.h"
#include "pch.h"

#include "Core/Application/Application.h"

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

    Renderer3D::Renderer3D(Math::FreeCamera *camera)
        : m_Camera(camera), m_ShadowMap(new ShadowMap()),
          m_ShadowMapMaterial(std::make_shared<Material>(Shader::CreateFromSource("Shadow", s_ShadowShader)))
    {}

    void Renderer3D::RenderEntities(World &world, Math::FreeCamera *camera, LightSystem &lightSystem,
                                    AssetManager &assets)
    {
        // Draw ModelComponent
        const auto &modelPool = world.GetPool<ModelComponent>();
        for (const auto &modelComponent : modelPool.GetComponents())
        {
            Model *model = assets.GetModel(modelComponent.m_ModelID);
            for (auto mesh : model->GetMeshes())
            {
                Ref<Material> mat = mesh->GetMaterial();
                mat->Bind();
                mat->SetUniform("u_LightCamera", lightSystem.m_Directional.m_LightCamera);
                mat->SetUniform("u_LightDir", lightSystem.m_Directional.m_Direction);
                mat->SetUniform("u_Model", modelComponent.m_Transformation.m_Model);
                mat->SetUniform("u_Camera", camera->GetCameraMatrix());
                mat->SetUniform("u_CameraPos", camera->GetCameraPos());
                mesh->DrawGeometry();
            }
        }
    }

    void Renderer3D::RenderEntitiesWithMaterial(World &world, Ref<Material> material, AssetManager &assets)
    {
        material->Bind();

        // Draw ModelComponents with custom material
        const auto &modelPool = world.GetPool<ModelComponent>();
        for (const auto &modelComponent : modelPool.GetComponents())
        {
            material->SetUniform("u_Model", modelComponent.m_Transformation.m_Model);
            Model *model = assets.GetModel(modelComponent.m_ModelID);
            model->DrawGeometry();
        }
    }

    void Renderer3D::DrawScene(Scene *scene)
    {
        RenderEntities(scene->GetWorld(), m_Camera, scene->m_LightSystem, *Application::s_Application->m_AssetManager);
        if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
    }

    void Renderer3D::DrawShadowedScene(Scene *scene)
    {

        // Bind shadowmap
        m_ShadowMap->Bind();

        // Draw to shadowmap
        Application::s_Application->m_RenderAPI->Culling(true, false);
        m_ShadowMapMaterial->BindShader();
        m_ShadowMapMaterial->SetUniform("u_LightCamera", scene->m_LightSystem.m_Directional.m_LightCamera);
        World &world = scene->GetWorld();
        RenderEntitiesWithMaterial(world, m_ShadowMapMaterial, *Application::s_Application->m_AssetManager);
        Application::s_Application->m_RenderAPI->Culling(Application::s_Application->m_RenderAPI->m_CullingDefault,
                                                         true);

        // Bind postfx render target
        m_PostEffect->Bind();

        // Bind shadowmap to index 3
        m_ShadowMap->BindTexture(3);
        DrawScene(scene);

        // Draw postfx to screen target
        m_PostEffect->Draw();
    }
} // namespace Dodo