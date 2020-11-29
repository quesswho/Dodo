#include "pch.h"
#include "Scene.h"
#include "Core/Application/Application.h"

namespace Dodo {

	Scene::Scene(Math::FreeCamera* camera, std::string name)
		: m_Name(name), m_Camera(camera), m_SkyBox(0) //Math::Mat4::Perspective(45.0f, Application::s_Application->m_WindowProperties.m_Width / Application::s_Application->m_WindowProperties.m_Height, 0.01f, 100.0f))
	{}

	Scene::~Scene()
	{}

	void Scene::CreateEntity(uint id, const std::string& name)
	{
		if (m_Entities.find(id) != m_Entities.end())
		{
			id = (uint)m_Entities.size();
			DD_WARN("Failed to give entity correct id: {}", name);
		}
		m_Entities.insert(std::make_pair(id, Entity(name)));
	}

	uint Scene::CreateEntity(const std::string& name)
	{
		uint result = (uint)m_Entities.size();
		m_Entities.insert(std::make_pair(result, Entity(name)));
		return result;
	}

	bool Scene::RenameEntity(uint id, const std::string& name)
	{
		auto it = m_Entities.find(id);
		if (it == m_Entities.end())
			return false;
		it->second.m_Name = name;
		return true;
	}

	bool Scene::DeleteEntity(uint id)
	{
		auto it = m_Entities.find(id);
		if (it == m_Entities.end())
			return false;
		m_Entities.erase(id);
		return true;
	}

	void Scene::Draw()
	{
		for (auto& ent : m_Entities)
		{
			for (auto i : ent.second.m_Drawable)
			{
				auto& drawable = ent.second.m_Components[i];
				switch (drawable.index())
				{
					case 0:
						std::get<0>(drawable)->Draw(m_Camera);
						break;
					case 1:
						std::get<1>(drawable)->Draw(m_Camera);
						break;
					default:
						DD_ERR("This should never occurr.");
						break;
				}
			}
		}

		if (m_SkyBox) m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}
}