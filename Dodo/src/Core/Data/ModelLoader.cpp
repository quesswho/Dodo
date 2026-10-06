#include "ModelLoader.h"

#include "AssetManager.h"
#include "Core/Graphics/Material/TextureSlot.h"

#include <algorithm>
#include <filesystem>

#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Dodo {

    ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& path)
    {
        DD_INFO("ModelLoader: loading '{}'", path);
        ModelData result;
        Assimp::Importer imp;
        const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DD_ERR("ModelLoader: unable to load '{}'", path);
            result.failed = true;
            return result;
        }

        // Tangents are generated in a second step so we know which meshes came with their own tangent frame.
        // Assimp's generated bitangent points along decreasing V, while a bitangent supplied by the file points
        // along increasing V (image up), so the two need opposite handedness signs.
        std::vector<bool> hasAuthoredTangents(scene->mNumMeshes);
        for (uint i = 0; i < scene->mNumMeshes; i++)
            hasAuthoredTangents[i] = scene->mMeshes[i]->HasTangentsAndBitangents();

        scene = imp.ApplyPostProcessing(aiProcess_CalcTangentSpace);
        if (!scene) {
            DD_ERR("ModelLoader: unable to generate tangents for '{}'", path);
            result.failed = true;
            return result;
        }

        std::filesystem::path fsPath(path);
        std::filesystem::path modelDir = fsPath.parent_path();

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

            aiString alphaModeStr;
            if (aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaModeStr) == AI_SUCCESS) {
                std::string_view mode = alphaModeStr.C_Str();
                if (mode == "BLEND")
                    matEntry.blendMode = BlendMode::AlphaBlend;
                else if (mode == "MASK")
                    matEntry.blendMode = BlendMode::AlphaCutout;
                else
                    matEntry.blendMode = BlendMode::Opaque;
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
                // Assimp UVs have their origin at the bottom-left, the engine uses top-left, so V is flipped.
                // Meshes without UVs have no texture coordinates and no tangent frame, fall back to defaults.
                if (aiM->HasTextureCoords(0))
                    v.m_Texcoord = {aiM->mTextureCoords[0][j].x, 1.0f - aiM->mTextureCoords[0][j].y};
                else
                    v.m_Texcoord = {0.0f, 0.0f};
                if (aiM->HasNormals())
                    v.m_Normal = {aiM->mNormals[j].x, aiM->mNormals[j].y, aiM->mNormals[j].z};
                else
                    v.m_Normal = {0.0f, 1.0f, 0.0f};
                if (aiM->HasTangentsAndBitangents()) {
                    // Store the bitangent handedness in tangent.w since we don't have a separate bitangent
                    // attribute. The shader rebuilds the bitangent as cross(N, T) * w, which must point image up
                    // (the direction of the normal map's green channel).
                    const aiVector3D imageUp = hasAuthoredTangents[i] ? aiM->mBitangents[j] : -aiM->mBitangents[j];
                    const aiVector3D crossNT = aiM->mNormals[j] ^ aiM->mTangents[j];
                    float bitangentSign = (crossNT * imageUp < 0.0f) ? -1.0f : 1.0f;
                    v.m_Tangent = {aiM->mTangents[j].x, aiM->mTangents[j].y, aiM->mTangents[j].z, bitangentSign};
                } else {
                    v.m_Tangent = {1.0f, 0.0f, 0.0f, 1.0f};
                }

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
