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

		ComponentType& FindComponent(int type)
		{
			for (auto& comp : m_Components)
			{
				if (type == static_cast<int>(comp.index()))
					return comp;
			}

			static ComponentType empty = std::monostate{};
			return empty;
		}

		
		const ComponentType& FindComponent(int type) const
		{
			for (const auto& comp : m_Components)
			{
				if (type == static_cast<int>(comp.index()))
					return comp;
			}

			static const ComponentType empty = std::monostate{};
			return empty;
		}

		std::string m_Name;
		ComponentFlag m_ComponentFlags;
	};
}