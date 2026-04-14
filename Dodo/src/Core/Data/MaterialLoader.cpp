#include "MaterialLoader.h"
#include "pch.h"

#include "AssetManager.h"

#include <assimp/material.h>

namespace Dodo {
    Ref<Material> MaterialLoader::LoadMaterial(const std::string& path, AssetManager& assets, RenderAPI& renderAPI)
    {
        ShaderID shaderID = assets.LoadShaderFromPath("res/shader/builtin/Passes/ForwardLit.slang");
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = shaderID;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        return std::make_shared<Material>(
            assets.GetPipeline(pipelineID), assets.GetTexture(assets.LoadTexture(path)),
            renderAPI.CreateSampler(SamplerProperties(SamplerWrapMode::WRAP_CLAMP_TO_EDGE)));
    }

    Ref<Material> MaterialLoader::LoadMaterial(const std::string& path, aiMaterial* aiMat, AssetManager& assets,
                                               RenderAPI& renderAPI)
    {
        MaterialFeatures flags = MaterialFeatures::None;
        std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

        Ref<Material> material = std::make_shared<Material>();
        uint numTextures = 0;

        // Albedo / base colour
        // Prefer aiTextureType_BASE_COLOR (glTF PBR), fall back to aiTextureType_DIFFUSE (OBJ/FBX)
        Ref<Texture> tex = LoadTextureFromMaterial(aiMat, aiTextureType_BASE_COLOR, flags, modelDir, assets);
        if (!tex) tex = LoadTextureFromMaterial(aiMat, aiTextureType_DIFFUSE, flags, modelDir, assets);
        if (tex) { material->AddTexture(0, tex); numTextures++; }

        // Roughness: slot 1, sample .g channel
        // Prefer separate roughness map; packed ORM (aiTextureType_GLTF_METALLIC_ROUGHNESS) is handled below
        tex = LoadTextureFromMaterial(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, flags, modelDir, assets);
        if (tex) { material->AddTexture(1, tex); numTextures++; }

        // Normal map: slot 2
        aiTextureType normalType = aiTextureType_NORMALS;
        {
            aiString str;
            if (aiMat->GetTexture(normalType, 0, &str) != AI_SUCCESS) normalType = aiTextureType_DISPLACEMENT;
        }
        tex = LoadTextureFromMaterial(aiMat, normalType, flags, modelDir, assets);
        if (tex) { material->AddTexture(2, tex); numTextures++; }

        // Slot 3: shadow map (bound externally by Renderer3D, not the material)

        // Metallic: slot 5, sample .b channel
        tex = LoadTextureFromMaterial(aiMat, aiTextureType_METALNESS, flags, modelDir, assets);
        if (tex) { material->AddTexture(5, tex); numTextures++; }

        // Packed ORM / glTF metallic-roughness: G = roughness, B = metallic
        // Only load if we don't already have separate maps
        if (!HasFeature(flags, MaterialFeatures::RoughnessMap) &&
            !HasFeature(flags, MaterialFeatures::MetallicMap))
        {
            tex = LoadTextureFromMaterial(aiMat, aiTextureType_GLTF_METALLIC_ROUGHNESS, flags, modelDir, assets);
            if (tex) {
                material->AddTexture(1, tex); // roughness — sample .g in shader
                material->AddTexture(5, tex); // metallic  — sample .b in shader
                numTextures++;
            }
        }

        // Ambient occlusion: slot 6
        tex = LoadTextureFromMaterial(aiMat, aiTextureType_AMBIENT_OCCLUSION, flags, modelDir, assets);
        if (!tex) tex = LoadTextureFromMaterial(aiMat, aiTextureType_LIGHTMAP, flags, modelDir, assets);
        if (tex) { material->AddTexture(6, tex); numTextures++; }

        if (numTextures == 0) {
            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                DD_WARN("Material {} has no textures!", name.C_Str());
            else
                DD_WARN("Material (unnamed) has no textures!");
            return std::make_shared<Material>(); // fallback
        }

        ShaderID shaderID = assets.LoadShaderFromPath("res/shader/builtin/Passes/ForwardLit.slang");
        PipelineDesc pipelineDesc;
        pipelineDesc.shaderID = shaderID;
        PipelineID pipelineID = assets.CreatePipeline(pipelineDesc, renderAPI);
        Ref<Pipeline> shader = assets.GetPipeline(pipelineID);
        if (!shader) DD_WARN("Could not create shader");

        material->SetShader(shader);
        material->SetSampler(renderAPI.CreateSampler(SamplerProperties()));

        return material;
    }

    Ref<Texture> MaterialLoader::LoadTextureFromMaterial(aiMaterial* material, int type, MaterialFeatures& features,
                                                         const std::filesystem::path& modelDir, AssetManager& assets)
    {
        aiTextureType typeEnum = static_cast<aiTextureType>(type);
        aiString str;
        if (!(material->GetTexture(typeEnum, 0, &str) == AI_SUCCESS && str.length > 0)) {
            return nullptr;
        }
        switch (type) {
        case aiTextureType_DIFFUSE:
        case aiTextureType_BASE_COLOR:
            features |= MaterialFeatures::AlbedoMap;
            break;
        case aiTextureType_NORMALS:
        case aiTextureType_DISPLACEMENT:
            features |= MaterialFeatures::NormalMap;
            break;
        case aiTextureType_DIFFUSE_ROUGHNESS:
            features |= MaterialFeatures::RoughnessMap;
            break;
        case aiTextureType_METALNESS:
            features |= MaterialFeatures::MetallicMap;
            break;
        case aiTextureType_GLTF_METALLIC_ROUGHNESS:
            features |= MaterialFeatures::MetallicMap | MaterialFeatures::RoughnessMap;
            break;
        case aiTextureType_AMBIENT_OCCLUSION:
        case aiTextureType_LIGHTMAP:
            features |= MaterialFeatures::AoMap;
            break;
        default:
            break;
        }
        std::string rawPath = str.C_Str();
        std::replace(rawPath.begin(), rawPath.end(), '\\', '/'); // Fix windows generated paths
        std::filesystem::path texturePath = modelDir / rawPath;

        DD_INFO("Texture: {}", texturePath.string());

        TextureID id = assets.LoadTexture(texturePath.string());
        return assets.GetTexture(id);
    }
} // namespace Dodo
