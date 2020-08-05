#pragma once

#include <string>

#include "Component/ModelComponent.h"

namespace Dodo {


	enum ComponentFlags
	{
		FlagNone = 0,
		FlagModelComponent = 1 << 0
	};
	DEFINE_ENUM_FLAG_OPERATORS(ComponentFlags);

	inline ComponentFlags& operator |= (ComponentFlags& a, int b) throw() { return (ComponentFlags&)(((_ENUM_FLAG_SIZED_INTEGER<ComponentFlags>::type&)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ComponentFlags>::type)b)); }

	struct Entity {
		Entity(const std::string& name)
			: m_Name(name), m_ComponentFlags()
		{}

		std::string m_Name;
		ComponentFlags m_ComponentFlags;
	};
}