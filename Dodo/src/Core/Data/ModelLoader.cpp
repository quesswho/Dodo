#include "ModelLoader.h"
#include "pch.h"

#include "AssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Dodo {

    ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& path, TextureLoader& textureLoader)
    {
        ModelData result;
        Assimp::Importer imp;
        const aiScene* scene =
            imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DD_WARN("ModelLoader: unable to load '{}'", path);
            result.failed = true;
            return result;
        }

        std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

        // Load materials
        for (uint i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* aiMat = scene->mMaterials[i];
            ModelData::MaterialEntry matEntry;

            auto tryLoadSlot = [&](int slot, aiTextureType type, MaterialFeatures feature) {
                aiString str;
                if (aiMat->GetTexture(type, 0, &str) != AI_SUCCESS || str.length == 0) return;
                std::string rawPath = str.C_Str();
                std::replace(rawPath.begin(), rawPath.end(), '\\', '/');
                std::string fullPath = (modelDir / rawPath).string();
                TextureData pixels = textureLoader.Load(fullPath);
                if (!pixels.pixels.empty()) {
                    matEntry.features |= feature;
                    matEntry.textures.push_back({slot, fullPath, std::move(pixels)});
                }
            };

            tryLoadSlot(0, aiTextureType_DIFFUSE, MaterialFeatures::AlbedoMap);
            tryLoadSlot(1, aiTextureType_SPECULAR, MaterialFeatures::SpecularMap);

            // Normal map: NORMALS and DISPLACEMENT are the same thing
            aiString tmp;
            aiTextureType normalType = (aiMat->GetTexture(aiTextureType_NORMALS, 0, &tmp) == AI_SUCCESS)
                                           ? aiTextureType_NORMALS
                                           : aiTextureType_DISPLACEMENT;
            tryLoadSlot(2, normalType, MaterialFeatures::NormalMap);

            result.materials.push_back(std::move(matEntry));
        }

        // Load mesh data
        for (uint i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* aiM = scene->mMeshes[i];
            ModelData::MeshEntry meshEntry;
            meshEntry.materialIndex = aiM->mMaterialIndex;
            meshEntry.vertices.reserve(aiM->mNumVertices);
            meshEntry.indices.reserve(aiM->mNumFaces * 3);

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

    Ref<Model> ModelLoader::BuildModel(const ModelData& data, const std::vector<Ref<Material>>& materials)
    {
        std::vector<Ref<Mesh>> meshes;
        meshes.reserve(data.meshes.size());

        for (const auto& meshEntry : data.meshes) {
            Ref<Material> mat = meshEntry.materialIndex < materials.size()
                                    ? materials[meshEntry.materialIndex]
                                    : std::make_shared<Material>();

            meshes.push_back(std::make_shared<Mesh>(
                new VertexBuffer(
                    (const float*)meshEntry.vertices.data(),
                    (uint)(meshEntry.vertices.size() * sizeof(Vertex)),
                    BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 3}})),
                new IndexBuffer(meshEntry.indices.data(), (uint)meshEntry.indices.size()), mat));
        }

        return std::make_shared<Model>(meshes);
    }

    Ref<Mesh> ModelLoader::LoadMesh(aiMesh* mesh, Ref<Material> material)
    {
        std::vector<Vertex> vertices;
        std::vector<uint> indices;

        size_t totalVertices = mesh->mNumVertices;
        size_t totalIndices = mesh->mNumFaces * 3; // Always 3 due to aiProcess_Triangulate

        // Reserve memory for indices and vertices
        vertices.reserve(totalVertices);
        indices.reserve(totalIndices);

        // Populate vertices
        for (uint j = 0; j < totalVertices; j++) {
            Vertex vertex;
            vertex.m_Position.x = mesh->mVertices[j].x;
            vertex.m_Position.y = mesh->mVertices[j].y;
            vertex.m_Position.z = mesh->mVertices[j].z;

            vertex.m_Texcoord.x = mesh->mTextureCoords[0][j].x;
            vertex.m_Texcoord.y = mesh->mTextureCoords[0][j].y;

            vertex.m_Normal.x = mesh->mNormals[j].x;
            vertex.m_Normal.y = mesh->mNormals[j].y;
            vertex.m_Normal.z = mesh->mNormals[j].z;
            vertex.m_Tangent.x = mesh->mTangents[j].x;
            vertex.m_Tangent.y = mesh->mTangents[j].y;
            vertex.m_Tangent.z = mesh->mTangents[j].z;

            vertices.emplace_back(std::move(vertex));
        }

        // Populate indices
        for (uint k = 0; k < mesh->mNumFaces; k++) {
            aiFace face = mesh->mFaces[k];
            for (uint j = 0; j < face.mNumIndices; j++) // mNumIndices = 3, by aiProcess_Triangulate flag
                indices.push_back(face.mIndices[j]);
        }

        return std::make_shared<Mesh>(
            new VertexBuffer((float*)&vertices[0], totalVertices * sizeof(Vertex),
                             BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 3}})),
            new IndexBuffer(indices.data(), totalIndices), material);
    }
} // namespace Dodo
