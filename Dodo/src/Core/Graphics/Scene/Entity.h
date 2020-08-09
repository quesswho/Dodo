#pragma once

#include "Component/Components.h"
#include "Component/ModelComponent.h"

namespace Dodo {


	struct Entity {
		Entity(const std::string& name)
			: m_Name(name), m_ComponentFlags()
		{}

		std::string m_Name;
		ComponentFlag m_ComponentFlags;
	};
}