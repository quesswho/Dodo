#include "ModelLoader.h"

#include "AssetManager.h"
#include "Core/Graphics/Material/TextureSlot.h"

#include <algorithm>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Dodo {

    ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& path)
    {
        DD_INFO("ModelLoader: loading '{}'", path);
        ModelData result;
        Assimp::Importer imp;
        const aiScene* scene =
            imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DD_ERR("ModelLoader: unable to load '{}'", path);
            result.failed = true;
            return result;
        }

        std::filesystem::path fsPath(path);
        std::filesystem::path modelDir = fsPath.parent_path();
        std::string ext = fsPath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        const bool isFbx = (ext == ".fbx");

        // Collect material texture paths. Texture loading is done async in AssetManager
        for (uint i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* aiMat = scene->mMaterials[i];
            ModelData::MaterialEntry matEntry;

            auto tryAddSlot = [&](int slot, aiTextureType type, MaterialFeatures feature) {
                aiString str;
                if (aiMat->GetTexture(type, 0, &str) != AI_SUCCESS || str.length == 0) return;
                std::string rawPath = str.C_Str();
                std::replace(rawPath.begin(), rawPath.end(), '\\', '/');
                std::string fullPath = (modelDir / rawPath).string();
                matEntry.features |= feature;
                matEntry.textures.push_back({slot, std::move(fullPath)});
            };

            // Albedo: BASE_COLOR (glTF PBR) with fallback to DIFFUSE (OBJ/FBX)
            {
                aiString str;
                aiTextureType albedoType =
                    (aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &str) == AI_SUCCESS && str.length > 0)
                        ? aiTextureType_BASE_COLOR
                        : aiTextureType_DIFFUSE;
                tryAddSlot((uint)TextureSlot::Albedo, albedoType, MaterialFeatures::AlbedoMap);
            }

            // If no albedo texture was found, fall back to a solid color from the material
            if (!HasFeature(matEntry.features, MaterialFeatures::AlbedoMap)) {
                aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
                if (aiMat->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS) aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                matEntry.albedoColor = {color.r, color.g, color.b, color.a};
            }

            tryAddSlot((uint)TextureSlot::Roughness, aiTextureType_DIFFUSE_ROUGHNESS, MaterialFeatures::RoughnessMap);

            // Normal map: NORMALS and DISPLACEMENT are the same thing
            aiString normalTmp;
            aiTextureType normalType = (aiMat->GetTexture(aiTextureType_NORMALS, 0, &normalTmp) == AI_SUCCESS)
                                           ? aiTextureType_NORMALS
                                           : aiTextureType_DISPLACEMENT;
            tryAddSlot((uint)TextureSlot::Normal, normalType, MaterialFeatures::NormalMap);

            // Spec-gloss workflow: only attempt when no dedicated roughness map was found,
            // since metallic-roughness assets may also export an aiTextureType_SPECULAR slot.
            if (!HasFeature(matEntry.features, MaterialFeatures::RoughnessMap))
                tryAddSlot((uint)TextureSlot::Spec, aiTextureType_SPECULAR, MaterialFeatures::SpecularMap);

            tryAddSlot((uint)TextureSlot::Metallic, aiTextureType_METALNESS, MaterialFeatures::MetallicMap);

            // Packed ORM (glTF metallic-roughness): G = roughness, B = metallic. Only use if
            // separate maps were not found, to avoid double-loading.
            if (!HasFeature(matEntry.features, MaterialFeatures::RoughnessMap) &&
                !HasFeature(matEntry.features, MaterialFeatures::MetallicMap)) {
                aiString ormTmp;
                if (aiMat->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &ormTmp) == AI_SUCCESS &&
                    ormTmp.length > 0) {
                    std::string rawPath = ormTmp.C_Str();
                    std::replace(rawPath.begin(), rawPath.end(), '\\', '/');
                    std::string fullPath = (modelDir / rawPath).string();
                    matEntry.features |= MaterialFeatures::RoughnessMap | MaterialFeatures::MetallicMap;
                    matEntry.textures.push_back({(uint)TextureSlot::Roughness, fullPath});
                    matEntry.textures.push_back({(uint)TextureSlot::Metallic, std::move(fullPath)});
                }
            }

            // AO: AMBIENT_OCCLUSION with fallback to LIGHTMAP (some exporters use LIGHTMAP for AO)
            {
                aiString aoTmp;
                aiTextureType aoType =
                    (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoTmp) == AI_SUCCESS && aoTmp.length > 0)
                        ? aiTextureType_AMBIENT_OCCLUSION
                        : aiTextureType_LIGHTMAP;
                tryAddSlot((uint)TextureSlot::Ao, aoType, MaterialFeatures::AoMap);
            }

            // Warn about any texture types present in the material that we do not handle
            {
                static const std::unordered_set<int> s_HandledTypes = {
                    aiTextureType_NONE,
                    aiTextureType_DIFFUSE,
                    aiTextureType_SPECULAR,
                    aiTextureType_NORMALS,
                    aiTextureType_DISPLACEMENT,
                    aiTextureType_LIGHTMAP,
                    aiTextureType_BASE_COLOR,
                    aiTextureType_DIFFUSE_ROUGHNESS,
                    aiTextureType_METALNESS,
                    aiTextureType_AMBIENT_OCCLUSION,
                    aiTextureType_GLTF_METALLIC_ROUGHNESS,
                };
                const char* matName = aiMat->GetName().C_Str();
                for (int t = aiTextureType_NONE; t <= AI_TEXTURE_TYPE_MAX; ++t) {
                    if (s_HandledTypes.count(t)) continue;
                    uint count = aiMat->GetTextureCount(static_cast<aiTextureType>(t));
                    if (count > 0) {
                        DD_WARN("ModelLoader: material '{}' has {} texture(s) of unhandled type '{}' ({}), ignoring",
                                matName, count, aiTextureTypeToString(static_cast<aiTextureType>(t)), t);
                    }
                }
            }

            result.materials.push_back(std::move(matEntry));
        }

        // Load mesh data
        for (uint i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* aiM = scene->mMeshes[i];
            ModelData::MeshEntry meshEntry;
            meshEntry.materialIndex = aiM->mMaterialIndex;
            meshEntry.vertices.reserve(aiM->mNumVertices);
            meshEntry.indices.reserve(aiM->mNumFaces *
                                      3); // Num indices is always a multiple of 3 because of aiProcess_Triangulate

            for (uint j = 0; j < aiM->mNumVertices; j++) {
                Vertex v;
                v.m_Position = {aiM->mVertices[j].x, aiM->mVertices[j].y, aiM->mVertices[j].z};
                v.m_Texcoord = {aiM->mTextureCoords[0][j].x, aiM->mTextureCoords[0][j].y};
                float nFlip = isFbx ? -1.0f : 1.0f;
                v.m_Normal = {nFlip * aiM->mNormals[j].x, nFlip * aiM->mNormals[j].y, nFlip * aiM->mNormals[j].z};
                // Compute bitangent sign and store in tangent.w since we don't have a separate bitangent attribute.
                // Assimp guarantees tangents and bitangents are orthogonal to normals, so we can use the cross product
                // to determine handedness.
                aiVector3D crossNT = aiM->mTangents[j] ^ aiM->mNormals[j];
                float bitangentSign = (crossNT * aiM->mBitangents[j] < 0.0f) ? -1.0f : 1.0f;
                // FBX (UE Z-up): negate tangent.xyz along with normal to keep TBN frame right-handed.
                v.m_Tangent = {nFlip * aiM->mTangents[j].x, nFlip * aiM->mTangents[j].y, nFlip * aiM->mTangents[j].z, bitangentSign};

                meshEntry.vertices.push_back(v);
            }

            for (uint k = 0; k < aiM->mNumFaces; k++)
                for (uint j = 0; j < aiM->mFaces[k].mNumIndices; j++)
                    meshEntry.indices.push_back(aiM->mFaces[k].mIndices[j]);

            result.meshes.push_back(std::move(meshEntry));
        }
        DD_INFO("ModelLoader: Finished loading '{}'", path);
        return result;
    }

    Ref<Model> ModelLoader::BuildModel(const ModelData& data, const std::vector<Ref<Material>>& materials,
                                       RenderAPI& renderAPI)
    {
        std::vector<Ref<Mesh>> meshes;
        meshes.reserve(data.meshes.size());

        for (const auto& meshEntry : data.meshes) {
            // Assimp should cover the out of bounds, but we cover it just in case
            Ref<Material> mat;
            if (meshEntry.materialIndex < materials.size()) {
                mat = materials[meshEntry.materialIndex];
            } else {
                mat = std::make_shared<Material>();
            }

            meshes.push_back(std::make_shared<Mesh>(
                renderAPI.CreateVertexBuffer(
                    (const float*)meshEntry.vertices.data(), (uint)(meshEntry.vertices.size() * sizeof(Vertex)),
                    BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 4}})),
                renderAPI.CreateIndexBuffer(meshEntry.indices.data(), (uint)meshEntry.indices.size()), mat));
        }

        return std::make_shared<Model>(meshes);
    }
} // namespace Dodo
