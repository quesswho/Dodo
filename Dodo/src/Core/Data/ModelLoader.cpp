#include "ModelLoader.h"
#include "pch.h"

#include "AssetManager.h"

#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Dodo {

    ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& path)
    {
        ModelData result;
        Assimp::Importer imp;
        const aiScene* scene =
            imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DD_ERR("ModelLoader: unable to load '{}'", path);
            result.failed = true;
            return result;
        }

        std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

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

            tryAddSlot(0, aiTextureType_DIFFUSE, MaterialFeatures::AlbedoMap);

            // Roughness: prefer dedicated PBR slot, fall back to specular
            aiString roughnessTmp;
            aiTextureType roughnessType =
                (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTmp) == AI_SUCCESS)
                    ? aiTextureType_DIFFUSE_ROUGHNESS
                    : aiTextureType_SPECULAR;
            tryAddSlot(1, roughnessType, MaterialFeatures::RoughnessMap);

            // Normal map: NORMALS and DISPLACEMENT are the same thing
            aiString normalTmp;
            aiTextureType normalType = (aiMat->GetTexture(aiTextureType_NORMALS, 0, &normalTmp) == AI_SUCCESS)
                                           ? aiTextureType_NORMALS
                                           : aiTextureType_DISPLACEMENT;
            tryAddSlot(2, normalType, MaterialFeatures::NormalMap);

            tryAddSlot(5, aiTextureType_METALNESS, MaterialFeatures::MetallicMap);
            tryAddSlot(6, aiTextureType_AMBIENT_OCCLUSION, MaterialFeatures::AoMap);

            aiString alphaMode;
            if (aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
                if (strcmp(alphaMode.C_Str(), "BLEND") == 0) matEntry.blendMode = BlendMode::AlphaBlend;
                // "MASK": stays Opaque (shader discards alpha < 0.001, sufficient for binary alpha masks)
                // "OPAQUE": default, no action needed
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
                v.m_Normal = {aiM->mNormals[j].x, aiM->mNormals[j].y, aiM->mNormals[j].z};
                v.m_Tangent = {aiM->mTangents[j].x, aiM->mTangents[j].y, aiM->mTangents[j].z};
                meshEntry.vertices.push_back(v);
            }

            for (uint k = 0; k < aiM->mNumFaces; k++)
                for (uint j = 0; j < aiM->mFaces[k].mNumIndices; j++)
                    meshEntry.indices.push_back(aiM->mFaces[k].mIndices[j]);

            result.meshes.push_back(std::move(meshEntry));
        }

        return result;
    }

    Ref<Model> ModelLoader::BuildModel(const ModelData& data, const std::vector<Ref<Material>>& materials,
                                       RenderAPI& renderAPI)
    {
        std::vector<Ref<Mesh>> meshes;
        meshes.reserve(data.meshes.size());

        for (const auto& meshEntry : data.meshes) {
            Ref<Material> mat = meshEntry.materialIndex < materials.size() ? materials[meshEntry.materialIndex]
                                                                           : std::make_shared<Material>();

            meshes.push_back(std::make_shared<Mesh>(
                renderAPI.CreateVertexBuffer(
                    (const float*)meshEntry.vertices.data(), (uint)(meshEntry.vertices.size() * sizeof(Vertex)),
                    BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 3}})),
                renderAPI.CreateIndexBuffer(meshEntry.indices.data(), (uint)meshEntry.indices.size()), mat));
        }

        return std::make_shared<Model>(meshes);
    }
} // namespace Dodo
