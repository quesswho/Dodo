#pragma once

#include "Core/Graphics/Material/Material.h"

#include <assimp/material.h>

namespace Dodo {
	
	class MaterialLoader
	{
	public:
		Ref<Material> LoadMaterial(const char* texture);
		Ref<Material> LoadMaterial(aiMaterial* material);
	};
}