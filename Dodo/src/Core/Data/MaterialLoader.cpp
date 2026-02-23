#include "pch.h"
#include "MaterialLoader.h"

#include "Core/Application/Application.h"

namespace Dodo
{
	Ref<Material> MaterialLoader::LoadMaterial(const char* path)
	{
		return std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), std::make_shared<Texture>(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_EDGE)));
	}

	Ref<Material> MaterialLoader::LoadMaterial(aiMaterial* material)
	{
		ShaderBuilderFlags flags = ShaderBuilderFlagShadowMap;

		std::vector<Ref<Texture>> textures;
		aiString str;

		material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
		if (str.length > 0)
		{
			flags |= ShaderBuilderFlagDiffuseMap;
			textures.push_back(std::make_shared<Texture>(str.C_Str(), 0));
			DD_INFO("Diffuse map: {0}", str.C_Str());
		}
		str = "";
		material->GetTexture(aiTextureType_SPECULAR, 0, &str);
		if (str.length > 0)
		{
			flags |= ShaderBuilderFlagSpecularMap;
			textures.push_back(std::make_shared<Texture>(str.C_Str(), (uint)textures.size()));
			DD_INFO("Specular map: {0}", str.C_Str());
		}

		str = "";
		material->GetTexture(aiTextureType_NORMALS, 0, &str);
		// NORMALS and DISPLACEMENT is the same thing
		if (str.length == 0) material->GetTexture(aiTextureType_DISPLACEMENT, 0, &str);

		if (str.length > 0)
		{
			flags |= ShaderBuilderFlagNormalMap;
			textures.push_back(std::make_shared<Texture>(str.C_Str(), (uint)textures.size()));
			DD_INFO("Normal map: {0}", str.C_Str());
		}


		if (textures.size() > 0) {
			Ref<Shader> shader = Application::s_Application->m_AssetManager->GetShader(flags);
			if (!shader) {
				DD_WARN("Could not create Shader");
			}
			return std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(flags), textures);
		}
		else {
			aiString name;
			if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
				DD_WARN("Material {0} does not have any textures!", name.C_Str());
			} else {
				DD_WARN("Material (unnamed) does not have any textures!");
			}
			return std::make_shared<Material>(); // Fallback shader
		}
	}
}