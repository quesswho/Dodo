#include "World.h"

namespace Dodo {

    
    void World::CreateEntity(uint id, const std::string& name)
	{
		if (m_Entities.find(id) != m_Entities.end())
		{
			id = (uint)m_Entities.size();
			DD_WARN("Failed to give entity correct id: {}", name);
		}
		m_Entities.emplace(std::make_pair(id, Entity(name)));
	}
    
	uint World::CreateEntity(const std::string& name)
	{
        // TODO: Use UUID instead
		uint result = (uint)m_Entities.size();
		m_Entities.emplace(std::make_pair(result, Entity(name)));
		return result;
	}
    
	bool World::RenameEntity(uint id, const std::string& name)
	{
        auto it = m_Entities.find(id);
		if (it == m_Entities.end())
        return false;
		it->second.m_Name = name;
		return true;
	}
    
	bool World::DeleteEntity(uint id)
	{
        auto it = m_Entities.find(id);
		if (it == m_Entities.end())
        return false;
		m_Entities.erase(id);
		return true;
	}
}