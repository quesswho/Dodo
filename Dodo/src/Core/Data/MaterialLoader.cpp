#include "pch.h"
#include "MaterialLoader.h"
#include "Core/Application/Application.h"

namespace Dodo
{
	Material* MaterialLoader::LoadMaterial(const char* path)
	{
		return new Material(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), new Texture(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_EDGE)));
	}
}