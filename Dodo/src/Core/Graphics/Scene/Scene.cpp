#include "pch.h"
#include "Scene.h"

namespace Dodo {

	Scene::Scene()
		: m_AmountOfComponents(1)
	{}

	Scene::~Scene()
	{}

	uint Scene::CreateEntity(const std::string& name)
	{
		uint result = m_Entities.size();
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
			model.second->Draw();
		}
	}
}