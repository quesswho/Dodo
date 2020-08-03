#pragma once
#include "Core/Common.h"
#include "Core/Graphics/ShaderBuilder.h"
#include "Core/Graphics/Scene/Model.h"
#include <unordered_map>

#include "ModelLoader.h"

namespace Dodo {

	class AssetManager {
	private:
		std::unordered_map<ShaderBuilderFlags, Shader*> m_ShaderBuilderShaders;  // Stores all shaders created by shaderbuilder

		std::unordered_map<uint, Model*> m_Models;		 // Stores id as key and model pointer as value
		std::unordered_map<std::string, uint> m_ModelID; // Stores path as key and id as value
	public:
		AssetManager();
		~AssetManager();

		Shader* GetShader(ShaderBuilderFlags flags);
		Model* GetModel(const char* path);

		Model* CreateModel(const char* path, uint id);
	public:
		ModelLoader* m_ModelLoader;
	};
}