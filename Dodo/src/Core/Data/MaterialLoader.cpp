#include "pch.h"
#include "MaterialLoader.h"
#include "Core/Application/Application.h"

namespace Dodo
{
	// TODO: Move code from Modelloader
	Material* MaterialLoader::LoadMaterial(const char* path)
	{
		return new Material(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), std::make_shared<Texture>(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_EDGE)));
	}
}