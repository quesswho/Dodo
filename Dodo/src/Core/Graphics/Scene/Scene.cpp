#include "pch.h"
#include "Scene.h"

namespace Dodo {

	Scene::Scene()
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
}