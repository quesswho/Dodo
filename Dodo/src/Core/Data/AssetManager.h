#pragma once
#include "Core/Common.h"
#include "Core/Graphics/ShaderBuilder.h"
#include <unordered_map>

namespace Dodo {

	class AssetManager {
	private:
		std::unordered_map<ShaderBuilderFlags, Shader*> m_ShaderBuilderShaders;
	public:
		AssetManager();
		~AssetManager();

		Shader* GetShader(ShaderBuilderFlags flags);
	};
}