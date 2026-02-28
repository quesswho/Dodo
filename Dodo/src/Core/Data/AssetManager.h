#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Shader/ShaderBuilder.h"
#include "MaterialLoader.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Graphics/Scene/Rectangle.h"
#include "Core/Graphics/Scene/Mesh/MeshFactory.h"
#include "Core/Graphics/CubeMap.h"
#include "ModelLoader.h"

#include <map>

namespace Dodo {

    
    using MaterialID = uint64_t;

	class AssetManager {
	private:
		std::unordered_map<ShaderBuilderFlags, Ref<Shader>> m_ShaderBuilderShaders;  // Stores all shaders created by shaderbuilder

		std::unordered_map<MaterialID, Ref<Material>> m_Materials;
		std::unordered_map<std::string, uint> m_MaterialID;

		std::unordered_map<ModelID, Model*> m_Models;		 // Stores id as key and model pointer as value
		std::unordered_map<std::string, uint> m_ModelID; // Stores path as key and id as value
        std::unordered_map<ModelID, std::string> m_ModelPath; // Stores id as key and path as value
	public:
		AssetManager(bool serialization);
		~AssetManager();

		Ref<Shader> GetShader(ShaderBuilderFlags flags);
		Ref<Material> GetMaterial(const char* path);

        ModelID LoadModel(const std::string& path);
        Model* GetModel(ModelID id);

        const std::string& GetModelPath(ModelID id);
	public:
		ModelLoader* m_ModelLoader;
		MaterialLoader* m_MaterialLoader;
		MeshFactory* m_MeshFactory;
	private:
		bool m_Serialization;
	};
}