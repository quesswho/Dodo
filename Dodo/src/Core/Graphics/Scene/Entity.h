#pragma once

#include <string>

namespace Dodo {


	enum struct Components
	{
		MeshComponent = 1 << 0
	};

	struct Entity {
		Entity(const std::string& name)
			: m_Name(name), m_ComponentFlags()
		{}

		std::string m_Name;
		Components m_ComponentFlags;
	};
}