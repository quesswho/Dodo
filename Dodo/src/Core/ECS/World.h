#pragma once

#include <Core/ECS/Entity.h>
#include <Core/ECS/Component/ModelComponent.h>
#include <Core/ECS/Component/Rectangle2DComponent.h>

#include <unordered_map>

namespace Dodo {
    class World {
    public:
        std::unordered_map<uint, Entity> m_Entities;

        World() = default;
        ~World() = default;

        void CreateEntity(uint id, const std::string& name);
		uint CreateEntity(const std::string& name = "Unnamed");
		bool RenameEntity(uint id, const std::string& name); // true ? Success : No entity with specified id
		bool DeleteEntity(uint id);

		template<typename T>
		void AddComponent(uint id, T comp)
		{
			auto it = m_Entities.find(id);
			if (it != m_Entities.end())
			{
				int i = 0;
				for (auto& c : (*it).second.m_Components)
				{
					if (c.index() == comp->GetIndex())
					{
						(*it).second.m_Components.erase((*it).second.m_Components.begin() + i);
						delete std::get<T>(c);
						break;
					}
					i++;
				}
				(*it).second.m_ComponentFlags |= comp->GetFlagType();
				(*it).second.m_Components.emplace_back(comp);
				if (comp->IsDrawable()) (*it).second.m_Drawable.emplace_back((*it).second.m_Components.size()-1); // Add the component to the drawable list
			}
		}
    };
}