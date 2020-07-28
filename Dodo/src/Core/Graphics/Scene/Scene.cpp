#include "pch.h"
#include "Scene.h"
#include "Core/Application/Application.h"

namespace Dodo {

	Scene::Scene(Math::FreeCamera* camera)
		: m_AmountOfComponents(1), m_Camera(camera) //Math::Mat4::Perspective(45.0f, Application::s_Application->m_WindowProperties.m_Width / Application::s_Application->m_WindowProperties.m_Height, 0.01f, 100.0f))
	{}

	Scene::~Scene()
	{}

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
			it->second = comp;
		m_ModelComponent.insert(std::make_pair(id, comp));
	}

	void Scene::Draw()
	{
		for (auto& model : m_ModelComponent)
		{
			model.second->Draw(m_Camera);
		}
	}
}