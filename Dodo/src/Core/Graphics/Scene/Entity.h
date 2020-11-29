#pragma once

#include "Component/Components.h"
#include "Component/ModelComponent.h"
#include "Component/Rectangle2DComponent.h"

#include <variant>

namespace Dodo {


	struct Entity {
		Entity(const std::string& name)
			: m_Name(name), m_ComponentFlags()
		{}
		using ComponentType = std::variant<ModelComponent*, Rectangle2DComponent*, std::monostate>;

		std::vector<ComponentType> m_Components; // Order needs to be the same as in the ComponentFlag enum!

		std::vector<short> m_Drawable; // Position of all drawables in vector<variant<...>>

		ComponentType FindComponent(int type) // GetIndex()
		{
			for (int i = 0; i < m_Components.size(); i++)
			{
				ComponentType comp = m_Components[i];
				if (type == comp.index())
					return comp;
			}
			return std::monostate {};
		}

		std::string m_Name;
		ComponentFlag m_ComponentFlags;
	};
}