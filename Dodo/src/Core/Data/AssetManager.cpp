#include "pch.h"
#include "AssetManager.h"
#include "Core/Application/Application.h"

namespace Dodo {

	AssetManager::AssetManager(bool serialization)
		: m_Serialization(serialization), m_ModelLoader(new ModelLoader), m_MaterialLoader(new MaterialLoader), m_MeshFactory(new MeshFactory())
	{}

	AssetManager::~AssetManager()
	{
		for (auto shaderbuildershader : m_ShaderBuilderShaders)
			delete shaderbuildershader.second;
		for (auto model : m_Models)
			delete model.second;
	}

	Shader* AssetManager::GetShader(ShaderBuilderFlags flags)
	{
		if (m_ShaderBuilderShaders.find(flags) != m_ShaderBuilderShaders.end())
			return m_ShaderBuilderShaders[flags];

		m_ShaderBuilderShaders.insert(std::make_pair(flags, Application::s_Application->m_RenderAPI->m_ShaderBuilder->BuildVertexFragmentShader(flags)));
		return m_ShaderBuilderShaders[flags];
	}

	Model* AssetManager::CreateModel(const char* path, uint id)
	{
		if (m_ModelID.find(path) != m_ModelID.end())
		{
			DD_WARN("Trying to create model that already exists! {0} ID: {1}", path, id);
			return m_Models[m_ModelID.at(path)];
		}

		Model* model = m_ModelLoader->LoadModel(path);
		m_ModelID.emplace(path, m_Models.size());
		m_Models.emplace(m_Models.size(), model);
		return model;
	}

	Model* AssetManager::GetModel(const char* path)
	{
		if (m_ModelID.find(path) != m_ModelID.end())
			return m_Models[m_ModelID.at(path)];
		Model* model = m_ModelLoader->LoadModel(path);
		m_ModelID.emplace(path, m_Models.size());
		m_Models.emplace(m_Models.size(), model);
		return model;
	}

	Material* AssetManager::GetMaterial(const char* path)
	{
		if (m_MaterialID.find(path) != m_MaterialID.end())
			return m_Materials[m_MaterialID.at(path)];
		Material* mat = m_MaterialLoader->LoadMaterial(path);
		m_MaterialID.emplace(path, m_Models.size());
		m_Materials.emplace(m_Models.size(), mat);
		return mat;
	}
}