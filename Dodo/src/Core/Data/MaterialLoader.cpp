#include "pch.h"
#include "MaterialLoader.h"
#include "Core/Application/Application.h"
namespace Dodo
{

	Material* MaterialLoader::LoadMaterial(const char* path)
	{
		Texture* tex = new Texture(path);
		return new Material(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagDiffuseMap));
	}
}