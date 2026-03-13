#include "MaterialLoader.h"
#include "pch.h"

#include "AssetManager.h"

#include <assimp/material.h>

namespace Dodo {
    Ref<Material> MaterialLoader::LoadMaterial(const std::string& path, AssetManager& assets)
    {
        ShaderID id = assets.LoadShader(ShaderBuilderFlags::ShaderBuilderFlagBasicTexture);
        return std::make_shared<Material>(
            assets.GetShader(id), std::make_shared<Texture>(path),
            std::make_shared<TextureSampler>(SamplerProperties(SamplerWrapMode::WRAP_CLAMP_TO_EDGE)));
    }

    Ref<Material> MaterialLoader::LoadMaterial(const std::string& path, aiMaterial* aiMat, AssetManager& assets)
    {
        ShaderBuilderFlags flags = ShaderBuilderFlagShadowMap;
        std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

        Ref<Material> material = std::make_shared<Material>();
        uint slot = 0;

        // Diffuse map
        Ref<Texture> tex = LoadTextureFromMaterial(aiMat, aiTextureType_DIFFUSE, flags, modelDir);
        if (tex) material->AddTexture(slot++, tex);

        // Specular map
        tex = LoadTextureFromMaterial(aiMat, aiTextureType_SPECULAR, flags, modelDir);
        if (tex) material->AddTexture(slot++, tex);

        // Normal map — NORMALS and DISPLACEMENT are the same thing
        aiTextureType normalType = aiTextureType_NORMALS;
        aiString str;
        if (aiMat->GetTexture(normalType, 0, &str) != AI_SUCCESS) normalType = aiTextureType_DISPLACEMENT;
        tex = LoadTextureFromMaterial(aiMat, normalType, flags, modelDir);
        if (tex) material->AddTexture(slot++, tex);

        if (slot == 0) {
            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                DD_WARN("Material {} has no textures!", name.C_Str());
            else
                DD_WARN("Material (unnamed) has no textures!");
            return std::make_shared<Material>(); // fallback
        }

        ShaderID shaderID = assets.LoadShader(flags);
        Ref<Shader> shader = assets.GetShader(shaderID);
        if (!shader) DD_WARN("Could not create shader");

        material->SetShader(shader);
        material->SetSampler(std::make_shared<TextureSampler>(SamplerProperties()));

        return material;
    }

    Ref<Texture> MaterialLoader::LoadTextureFromMaterial(aiMaterial* material, int type,
                                                         ShaderBuilderFlags& shaderFlags,
                                                         const std::filesystem::path& modelDir)
    {
        aiTextureType typeEnum = static_cast<aiTextureType>(type);
        aiString str;
        if (!(material->GetTexture(typeEnum, 0, &str) == AI_SUCCESS && str.length > 0)) {
            return nullptr;
        }
        switch (type) {
        case aiTextureType_DIFFUSE:
            shaderFlags |= ShaderBuilderFlagDiffuseMap;
            break;
        case aiTextureType_SPECULAR:
            shaderFlags |= ShaderBuilderFlagSpecularMap;
            break;
        case aiTextureType_NORMALS:
        case aiTextureType_DISPLACEMENT:
            shaderFlags |= ShaderBuilderFlagNormalMap;
            break;
        default:
            break;
        }
        std::string rawPath = str.C_Str();
        std::replace(rawPath.begin(), rawPath.end(), '\\', '/'); /// Fix windows generated paths
        std::filesystem::path texturePath = modelDir / rawPath;

        DD_INFO("Texture: {}", texturePath.string());

        return std::make_shared<Texture>(texturePath.string().c_str());
    }
} // namespace Dodo