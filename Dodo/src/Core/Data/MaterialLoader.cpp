#include "pch.h"
#include "MaterialLoader.h"

#include "Core/Application/Application.h"

namespace Dodo
{
	// TODO: Move code from Modelloader
	Ref<Material> MaterialLoader::LoadMaterial(const char* path)
	{
		return std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), std::make_shared<Texture>(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_EDGE)));
	}
}