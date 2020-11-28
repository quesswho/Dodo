#pragma once

#include "Core/Graphics/Material/Material.h"

namespace Dodo {
	
	class MaterialLoader
	{
	public:
		Material* LoadMaterial(const char* texture);
	};
}