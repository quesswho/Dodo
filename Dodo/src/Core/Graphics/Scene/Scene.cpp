#include "pch.h"
#include "Scene.h"
#include "Core/Application/Application.h"

namespace Dodo {

	Scene::Scene(Math::FreeCamera* camera, std::string name)
		: m_AmountOfComponents(1), m_Name(name), m_Camera(camera), m_SkyBox(0) //Math::Mat4::Perspective(45.0f, Application::s_Application->m_WindowProperties.m_Width / Application::s_Application->m_WindowProperties.m_Height, 0.01f, 100.0f))
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

	void Scene::AddComponent(uint id, ModelComponent* comp)
	{
		auto it = m_ModelComponent.find(id);
		if (it != m_ModelComponent.end())
		{
			delete it->second;
			it->second = comp;
		}
		else
			m_ModelComponent.insert(std::make_pair(id, comp));
		m_Entities.at(id).m_ComponentFlags |= comp->GetFlagType();
	}

	void Scene::AddComponent(uint id, Rectangle2DComponent* comp)
	{
		auto it = m_Rectangle2DComponent.find(id);
		if (it != m_Rectangle2DComponent.end())
		{
			delete it->second;
			it->second = comp;
		}
		else
			m_Rectangle2DComponent.insert(std::make_pair(id, comp));
		m_Entities.at(id).m_ComponentFlags |= comp->GetFlagType();
	}

	void Scene::Draw()
	{
		for (auto& comp : m_ModelComponent)
		{
			comp.second->Draw(m_Camera);
		}

		for (auto& comp : m_Rectangle2DComponent)
		{
			comp.second->Draw(m_Camera);
		}

		if (m_SkyBox) m_SkyBox->Draw(m_Camera->GetViewMatrix());
	}
}