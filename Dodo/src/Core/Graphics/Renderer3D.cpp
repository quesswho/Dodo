#include "pch.h"
#include "Renderer3D.h"

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

	Renderer3D::Renderer3D(Math::FreeCamera* camera)
		: m_Camera(camera), m_ShadowMap(new ShadowMap()), 
		m_ShadowMapMaterial(std::make_shared<Material>(Shader::CreateFromSource("Shadow", s_ShadowShader)))
	{}

	void Renderer3D::DrawScene(Scene* scene) {
		for (auto& ent : scene->m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
				case 0:
					std::get<0>(drawable)->Draw(m_Camera, scene->m_LightSystem);
					break;
				case 1:
					std::get<1>(drawable)->Draw(m_Camera, scene->m_LightSystem);
					break;
				default:
					DD_ERR("This should never occurr.");
					break;
				}
			}
		}

		if (scene->m_SkyBox) scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}

	void Renderer3D::DrawShadowedScene(Scene* scene) {

		// Bind shadowmap
		m_ShadowMap->Bind();

		// Draw to shadowmap
		Application::s_Application->m_RenderAPI->Culling(true, false);
		m_ShadowMapMaterial->BindShader();
		m_ShadowMapMaterial->SetUniform("u_LightCamera", scene->m_LightSystem.m_Directional.m_LightCamera);
		for (auto& ent : scene->m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
				case 0:
					std::get<0>(drawable)->Draw(m_ShadowMapMaterial);
					break;
				case 1:
					std::get<1>(drawable)->Draw(m_ShadowMapMaterial);
					break;
				default:
					DD_ERR("This should never occurr.");
					break;
				}
			}
		}
		Application::s_Application->m_RenderAPI->Culling(Application::s_Application->m_RenderAPI->m_CullingDefault, true);

		// Bind postfx render target
		m_PostEffect->Bind();

		// Bind shadowmap to index 3
		m_ShadowMap->BindTexture(3);
		DrawScene(scene);

		// Draw postfx to screen target
		m_PostEffect->Draw();
	}
}