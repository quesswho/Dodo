#include "pch.h"
#include "MaterialLoader.h"
#include "Core/Application/Application.h"

namespace Dodo
{
	Material* MaterialLoader::LoadMaterial(const char* path)
	{
		//std::string texPath = path;
		//texPath = texPath.substr(strlen("res/"), texPath.size()); // Remove res/ because we are already inside it
		//std::replace(texPath.begin(), texPath.end(), '/', '\\');
		return new Material(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), new Texture(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_BORDER)));
	}
}