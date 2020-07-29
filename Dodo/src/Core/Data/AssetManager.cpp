#include "pch.h"
#include "AssetManager.h"
#include "Core/Application/Application.h"

namespace Dodo {

	AssetManager::AssetManager()
	{

	}

	AssetManager::~AssetManager()
	{
		for (auto& shaderbuildershader : m_ShaderBuilderShaders)
			delete shaderbuildershader.second;
	}

	Shader* AssetManager::GetShader(ShaderBuilderFlags flags)
	{
		if (m_ShaderBuilderShaders.find(flags) != m_ShaderBuilderShaders.end())
			return m_ShaderBuilderShaders[flags];

		m_ShaderBuilderShaders.insert(std::make_pair(flags, Application::s_Application->m_RenderAPI->m_ShaderBuilder->BuildVertexFragmentShader(flags)));
		return m_ShaderBuilderShaders[flags];
	}
}