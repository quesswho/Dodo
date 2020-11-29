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
		
		std::vector<std::variant<ModelComponent*, Rectangle2DComponent*>> m_Components; // Order needs to be the same as in the ComponentFlag enum!

		std::vector<short> m_Drawable; // Position in vector

		std::string m_Name;
		ComponentFlag m_ComponentFlags;
	};
}