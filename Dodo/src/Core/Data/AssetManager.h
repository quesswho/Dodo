#pragma once
#include "Core/Common.h"
#include "Core/Graphics/Material/ShaderBuilder.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Graphics/Scene/Mesh/MeshFactory.h"
#include "Core/Graphics/CubeMap.h"
#include "ModelLoader.h"

#include <map>

namespace Dodo {

	class AssetManager {
	private:
		std::unordered_map<ShaderBuilderFlags, Shader*> m_ShaderBuilderShaders;  // Stores all shaders created by shaderbuilder

		std::unordered_map<uint, Model*> m_Models;		 // Stores id as key and model pointer as value
		std::unordered_map<std::string, uint> m_ModelID; // Stores path as key and id as value
	public:
		AssetManager(bool serialization);
		~AssetManager();

		Shader* GetShader(ShaderBuilderFlags flags);
		Model* GetModel(const char* path);

		Model* CreateModel(const char* path, uint id);

		Mesh* GetRectangle();
	public:
		ModelLoader* m_ModelLoader;
		MeshFactory* m_MeshFactory;
	private:
		// MeshFactory
		Mesh* m_Rectangle;

	private:
		bool m_Serialization;
	};
}