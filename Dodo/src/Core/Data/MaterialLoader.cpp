#include "pch.h"
#include "MaterialLoader.h"

#include "Core/Application/Application.h"

namespace Dodo
{
	Ref<Material> MaterialLoader::LoadMaterial(const char* path)
	{
		return std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture), std::make_shared<Texture>(path, 0, TextureSettings(TextureWrapMode::WRAP_CLAMP_TO_EDGE)));
	}

	Ref<Material> MaterialLoader::LoadMaterial(const std::string& path, aiMaterial* material)
	{
		ShaderBuilderFlags flags = ShaderBuilderFlagShadowMap;
		std::vector<Ref<Texture>> textures;

		std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

		// Diffuse map
		Ref<Texture> tex = LoadTextureFromMaterial(material, aiTextureType_DIFFUSE, flags, modelDir, textures.size());
		if (tex) textures.push_back(tex);

		// Specular map
		tex = LoadTextureFromMaterial(material, aiTextureType_SPECULAR, flags, modelDir, textures.size());
		if (tex) textures.push_back(tex);

		// NORMALS and DISPLACEMENT is the same thing
		aiTextureType normalType = aiTextureType_NORMALS;
		aiString str;
		if (material->GetTexture(normalType, 0, &str) != AI_SUCCESS)
			normalType = aiTextureType_DISPLACEMENT;

		tex = LoadTextureFromMaterial(material, normalType, flags, modelDir, textures.size());
		if (tex) textures.push_back(tex);

		// Create material
		if (!textures.empty()) {
			Ref<Shader> shader = Application::s_Application->m_AssetManager->GetShader(flags);
			if (!shader) {
				DD_WARN("Could not create Shader");
			}
			return std::make_shared<Material>(Application::s_Application->m_AssetManager->GetShader(flags), textures);
		}
		
		
		aiString name;
		if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
			DD_WARN("Material {0} does not have any textures!", name.C_Str());
		} else {
			DD_WARN("Material (unnamed) does not have any textures!");
		}
		return std::make_shared<Material>(); // Fallback shader
	
	}

	Ref<Texture> MaterialLoader::LoadTextureFromMaterial(aiMaterial* material,
                                                     aiTextureType type,
                                                     ShaderBuilderFlags& shaderFlags,
                                                     const std::filesystem::path& modelDir,
                                                     uint slot)
	{
		aiString str;
		if (material->GetTexture(type, 0, &str) == AI_SUCCESS && str.length > 0)
		{
			switch(type)
			{
				case aiTextureType_DIFFUSE:  shaderFlags |= ShaderBuilderFlagDiffuseMap; break;
				case aiTextureType_SPECULAR: shaderFlags |= ShaderBuilderFlagSpecularMap; break;
				case aiTextureType_NORMALS:
				case aiTextureType_DISPLACEMENT: shaderFlags |= ShaderBuilderFlagNormalMap; break;
				default: break;
			}
			std::string rawPath = str.C_Str();
			std::replace(rawPath.begin(), rawPath.end(), '\\', '/'); /// Fix windows generated paths
			std::filesystem::path texturePath = modelDir / rawPath;

			DD_INFO("Texture: {}", texturePath.string());

			return std::make_shared<Texture>(texturePath.string().c_str(), slot);
		}

		return nullptr;
	}
}