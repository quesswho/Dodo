#include "pch.h"
#include "Scene.h"

#include "Core/Application/Application.h"

namespace Dodo {

	Scene::Scene(Math::FreeCamera* camera, std::string name)
		: m_Name(name), m_Camera(camera), m_SkyBox(0), m_World(new World()) //Math::Mat4::Perspective(45.0f, Application::s_Application->m_WindowProperties.m_Width / Application::s_Application->m_WindowProperties.m_Height, 0.01f, 100.0f))
	{}

	Scene::~Scene()
	{
		delete m_World;
	}

    World& Scene::GetWorld() {
        return *m_World;
    }

	void Scene::Draw()
	{
		for (auto& ent : m_World->m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
					case 0:
						std::get<0>(drawable)->Draw(m_Camera, m_LightSystem);
						break;
					case 1:
						std::get<1>(drawable)->Draw(m_Camera, m_LightSystem);
						break;
					default:
						DD_ERR("This should never occurr.");
						break;
				}
			}
		}

		if (m_SkyBox) m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}

	void Scene::Draw(const Math::Mat4& lightCamera, Ref<Material> material) {
		Application::s_Application->m_RenderAPI->Culling(true, false);
		material->BindShader();
		material->SetUniform("u_LightCamera", lightCamera);
		for (auto& ent :  m_World->m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
				case 0:
					std::get<0>(drawable)->Draw(material);
					break;
				case 1:
					std::get<1>(drawable)->Draw(material);
					break;
				default:
					DD_ERR("This should never occurr.");
					break;
				}
			}
		}
		Application::s_Application->m_RenderAPI->Culling(Application::s_Application->m_RenderAPI->m_CullingDefault, true);
	}
}