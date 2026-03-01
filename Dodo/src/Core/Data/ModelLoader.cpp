#include "ModelLoader.h"
#include "pch.h"

#include "Core/Application/Application.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Dodo {

    Mesh *ModelLoader::LoadMesh(aiMesh *mesh, Ref<Material> material)
    {
        std::vector<Vertex> vertices;
        std::vector<uint> indices;

        size_t totalVertices = mesh->mNumVertices;
        size_t totalIndices = mesh->mNumFaces * 3; // Always 3 due to aiProcess_Triangulate

        // Reserve memory for indices and vertices
        vertices.reserve(totalVertices);
        indices.reserve(totalIndices);

        // Populate vertices
        for (uint j = 0; j < totalVertices; j++)
        {
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
        for (uint k = 0; k < mesh->mNumFaces; k++)
        {
            aiFace face = mesh->mFaces[k];
            for (uint j = 0; j < face.mNumIndices; j++) // mNumIndices = 3, by aiProcess_Triangulate flag
                indices.push_back(face.mIndices[j]);
        }

        return new Mesh(
            new VertexBuffer((float *)&vertices[0], totalVertices * sizeof(Vertex),
                             BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 3}})),
            new IndexBuffer(indices.data(), totalIndices), material);
    }

    Model *ModelLoader::LoadModel(const std::string &path)
    {
        Assimp::Importer imp;

        // Assimp::Importer has ownership of all the memory allocated from ReadFile()
        const aiScene *model =
            imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

        if (!model || model->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode)
        {
            DD_WARN("Unable to load model: {}", path);
            return nullptr; // TODO: Replace with error model
        }

        // Change work directory to get the textures
        // Application::s_Application->m_Window->TruncateWorkDirectory(path);

        std::vector<Ref<Material>> materials;
        materials.reserve(model->mNumMaterials);
        if (!model->HasMaterials())
        {
            materials.push_back(std::make_shared<Material>());
            DD_WARN("No materials found for {}", path);
        }

        for (int i = 0; i < model->mNumMaterials; i++)
        {
            materials.push_back(
                Application::s_Application->m_AssetManager->m_MaterialLoader->LoadMaterial(path, model->mMaterials[i]));
        }

        // Application::s_Application->m_Window->DefaultWorkDirectory();
        DD_INFO("{} materials loaded", materials.size());

        std::vector<Mesh *> meshes;
        meshes.reserve(model->mNumMeshes);

        for (uint i = 0; i < model->mNumMeshes; i++)
        {
            meshes.push_back(LoadMesh(model->mMeshes[i], materials[model->mMeshes[i]->mMaterialIndex]));
        }

        return new Model(meshes);
    }
} // namespace Dodo