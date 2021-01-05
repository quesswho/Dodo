#pragma once

#include "Component/Components.h"
#include "Component/ModelComponent.h"
#include "Component/Rectangle2DComponent.h"

#include <variant>

namespace Dodo {


	struct Entity {
		using ComponentType = std::variant<ModelComponent*, Rectangle2DComponent*, std::monostate>; // Order needs to be the same as in the ComponentFlag enum!
		
		Entity(const std::string& name)
			: m_Name(name), m_ComponentFlags()
		{}

		~Entity()
		{
			for (ComponentType& comp : m_Components)
				comp = std::monostate();
			m_Components.clear();
		}

		std::vector<ComponentType> m_Components;
		std::vector<short> m_Drawable; // Position of all drawables in vector<variant<...>>

		ComponentType FindComponent(int type) // The argument is the same as GetIndex() function inside all the components.
		{
			for (int i = 0; i < m_Components.size(); i++)
			{
				ComponentType comp = m_Components[i];
				if (type == comp.index())
					return comp;
			}
			return std::monostate {}; // Did not find a component.
		}

		std::string m_Name;
		ComponentFlag m_ComponentFlags;
	};
}