#include "MaterialLoader.h"

#include <assimp/material.h>
#include <tinyxml2.h>
#include <unordered_map>

namespace Dodo {

    MaterialLoader::MaterialData MaterialLoader::LoadMaterialData(const std::string& path)
    {
        if (!std::filesystem::is_directory(path)) {
            DD_ERR("{} is not a valid path!", path);
        }

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.path().extension() == ".mtlx") {
                MaterialData data = LoadMaterialDataFromMtlx(entry.path().string());
                if (data.valid) return data;
            }
        }

        MaterialData data;
        data.textures.push_back({0, path});
        data.samplerProps = SamplerProperties(SamplerWrapMode::WRAP_CLAMP_TO_EDGE);
        data.valid = true;
        return data;
    }

    MaterialLoader::MaterialData MaterialLoader::LoadMaterialDataFromMtlx(const std::string& mtlxPath)
    {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(mtlxPath.c_str()) != tinyxml2::XML_SUCCESS) {
            DD_WARN("Failed to parse MTLX file: {}", mtlxPath);
            return {};
        }

        tinyxml2::XMLElement* root = doc.FirstChildElement("materialx");
        if (!root) return {};

        std::filesystem::path dir = std::filesystem::path(mtlxPath).parent_path();

        const char* prefix = root->Attribute("fileprefix");
        std::string filePrefix = prefix ? prefix : "./";

        std::unordered_map<std::string, std::string> tiledImages;
        std::unordered_map<std::string, std::string> normalMapSources;

        for (tinyxml2::XMLElement* el = root->FirstChildElement(); el; el = el->NextSiblingElement()) {
            std::string tag = el->Name();
            const char* elName = el->Attribute("name");
            if (!elName) continue;

            if (tag == "tiledimage") {
                for (tinyxml2::XMLElement* inp = el->FirstChildElement("input"); inp;
                     inp = inp->NextSiblingElement("input")) {
                    const char* inpName = inp->Attribute("name");
                    const char* inpValue = inp->Attribute("value");
                    if (inpName && inpValue && std::string(inpName) == "file")
                        tiledImages[elName] = inpValue;
                }
            } else if (tag == "normalmap") {
                for (tinyxml2::XMLElement* inp = el->FirstChildElement("input"); inp;
                     inp = inp->NextSiblingElement("input")) {
                    const char* inpName = inp->Attribute("name");
                    const char* nodename = inp->Attribute("nodename");
                    if (inpName && nodename && std::string(inpName) == "in")
                        normalMapSources[elName] = nodename;
                }
            }
        }

        auto resolve = [&](const std::string& nodename) -> std::string {
            auto it = tiledImages.find(nodename);
            if (it != tiledImages.end())
                return (dir / (filePrefix + it->second)).lexically_normal().string();
            auto nmIt = normalMapSources.find(nodename);
            if (nmIt != normalMapSources.end()) {
                auto imgIt = tiledImages.find(nmIt->second);
                if (imgIt != tiledImages.end())
                    return (dir / (filePrefix + imgIt->second)).lexically_normal().string();
            }
            return {};
        };

        static const std::unordered_map<std::string, uint> kSlotMap = {
            {"base_color", 0},
            {"specular_roughness", 1},
            {"geometry_normal", 2},
        };

        tinyxml2::XMLElement* pbr = root->FirstChildElement("open_pbr_surface");
        if (!pbr) {
            DD_WARN("MTLX file has no open_pbr_surface node: {}", mtlxPath);
            return {};
        }

        MaterialData data;
        data.samplerProps = SamplerProperties(SamplerFilter::MIN_MAG_MIP_LINEAR);

        for (tinyxml2::XMLElement* inp = pbr->FirstChildElement("input"); inp;
             inp = inp->NextSiblingElement("input")) {
            const char* inpName = inp->Attribute("name");
            const char* nodename = inp->Attribute("nodename");
            if (!inpName || !nodename) continue;

            auto slotIt = kSlotMap.find(inpName);
            if (slotIt == kSlotMap.end()) continue;

            std::string texPath = resolve(nodename);
            if (texPath.empty()) {
                DD_WARN("MTLX: could not resolve node '{}' for input '{}'", nodename, inpName);
                continue;
            }

            DD_INFO("MTLX texture slot {}: {}", slotIt->second, texPath);
            data.textures.push_back({slotIt->second, texPath});
        }

        if (data.textures.empty()) {
            DD_WARN("MTLX material loaded no textures from: {}", mtlxPath);
            return {};
        }

        data.valid = true;
        return data;
    }

    MaterialLoader::MaterialData MaterialLoader::LoadMaterialData(const std::string& modelDir,
                                                                   aiMaterial* aiMat)
    {
        MaterialFeatures flags = MaterialFeatures::None;
        std::filesystem::path dir = modelDir;

        MaterialData data;
        data.samplerProps = SamplerProperties(SamplerFilter::MIN_MAG_MIP_LINEAR);

        auto addSlot = [&](int type, uint slot) {
            std::string path = GetTexturePath(aiMat, type, flags, dir);
            if (!path.empty())
                data.textures.push_back({slot, path});
        };

        // Albedo: slot 0
        addSlot(aiTextureType_BASE_COLOR, 0);
        if (data.textures.empty() || data.textures.back().slot != 0)
            addSlot(aiTextureType_DIFFUSE, 0);

        // Roughness: slot 1
        addSlot(aiTextureType_DIFFUSE_ROUGHNESS, 1);

        // Normal: slot 2
        {
            aiTextureType normalType = aiTextureType_NORMALS;
            aiString str;
            if (aiMat->GetTexture(normalType, 0, &str) != AI_SUCCESS)
                normalType = aiTextureType_DISPLACEMENT;
            addSlot(normalType, 2);
        }

        // Metallic: slot 5
        addSlot(aiTextureType_METALNESS, 5);

        // Packed ORM: slots 1 + 5 if no separate maps
        if (!HasFeature(flags, MaterialFeatures::RoughnessMap) &&
            !HasFeature(flags, MaterialFeatures::MetallicMap)) {
            std::string path = GetTexturePath(aiMat, aiTextureType_GLTF_METALLIC_ROUGHNESS, flags, dir);
            if (!path.empty()) {
                data.textures.push_back({1, path});
                data.textures.push_back({5, path});
            }
        }

        // AO: slot 6
        addSlot(aiTextureType_AMBIENT_OCCLUSION, 6);
        if (data.textures.empty() || data.textures.back().slot != 6)
            addSlot(aiTextureType_LIGHTMAP, 6);

        if (data.textures.empty()) {
            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                DD_WARN("Material {} has no textures!", name.C_Str());
            else
                DD_WARN("Material (unnamed) has no textures!");
            return data; // valid = false
        }

        data.valid = true;
        return data;
    }

    std::string MaterialLoader::GetTexturePath(aiMaterial* material, int type, MaterialFeatures& features,
                                                const std::filesystem::path& modelDir)
    {
        aiTextureType typeEnum = static_cast<aiTextureType>(type);
        aiString str;
        if (!(material->GetTexture(typeEnum, 0, &str) == AI_SUCCESS && str.length > 0))
            return {};

        switch (type) {
        case aiTextureType_DIFFUSE:
        case aiTextureType_BASE_COLOR:          features |= MaterialFeatures::AlbedoMap; break;
        case aiTextureType_NORMALS:
        case aiTextureType_DISPLACEMENT:        features |= MaterialFeatures::NormalMap; break;
        case aiTextureType_DIFFUSE_ROUGHNESS:   features |= MaterialFeatures::RoughnessMap; break;
        case aiTextureType_METALNESS:           features |= MaterialFeatures::MetallicMap; break;
        case aiTextureType_GLTF_METALLIC_ROUGHNESS:
            features |= MaterialFeatures::MetallicMap | MaterialFeatures::RoughnessMap;
            break;
        case aiTextureType_AMBIENT_OCCLUSION:
        case aiTextureType_LIGHTMAP:            features |= MaterialFeatures::AoMap; break;
        default: break;
        }

        std::string rawPath = str.C_Str();
        std::replace(rawPath.begin(), rawPath.end(), '\\', '/');
        std::filesystem::path texturePath = modelDir / rawPath;
        DD_INFO("Texture: {}", texturePath.string());
        return texturePath.string();
    }

} // namespace Dodo
