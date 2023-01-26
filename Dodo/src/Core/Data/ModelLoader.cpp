#include "pch.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Core/Application/Application.h"

namespace Dodo {


	Mesh* ModelLoader::LoadMesh(aiMesh* mesh, Material* material)
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

		return new Mesh(new VertexBuffer((float*)&vertices[0], totalVertices * sizeof(Vertex), 
			BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3 }, { "TANGENT", 3 } })), 
			new IndexBuffer(indices.data(), totalIndices),
			material);
	}

	Model* ModelLoader::LoadModel(const char* path)
	{
		Assimp::Importer imp;

		const aiScene* model = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

		if (!model || model->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode)
		{
			DD_WARN("Unable to load model: {}", path);
			return nullptr; // TODO: Replace with error model
		}
		
		// Change work directory to get the textures
		Application::s_Application->m_Window->TruncateWorkDirectory(path);

		std::vector<Material*> materials;
		materials.reserve(model->mNumMaterials);
		if (!model->HasMaterials())
		{
			materials.push_back(new Material());
			DD_WARN("No materials found for {}", path);
		}

		for (int i = 0; i < model->mNumMaterials; i++)
		{
			Material* material = nullptr;
			ShaderBuilderFlags flags = ShaderBuilderFlagNone;

			aiMaterial* mat = model->mMaterials[i];
			std::vector<Texture*> textures;
			aiString str;
			
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagDiffuseMap;
				textures.push_back(new Texture(str.C_Str(), 0));
				DD_INFO("Diffuse map: {0}", str.C_Str());
			}
			str = "";
			mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagSpecularMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
				DD_INFO("Specular map: {0}", str.C_Str());
			}

			str = "";
			mat->GetTexture(aiTextureType_NORMALS, 0, &str);
			// NORMALS and DISPLACEMENT is the same thing
			if (str.length == 0) mat->GetTexture(aiTextureType_DISPLACEMENT, 0, &str);
			
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagNormalMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
				DD_INFO("Normal map: {0}", str.C_Str());
			}


			if (textures.size() > 0) {
				Shader* shader = Application::s_Application->m_AssetManager->GetShader(flags);
				if (!shader) {
					DD_WARN("Could not create Shader");
				}
				material = new Material(Application::s_Application->m_AssetManager->GetShader(flags), textures);
			}
			else {
				material = new Material(); // Fallback shader
				DD_WARN("Material {0} does not have any textures: {1}", mat->GetEntryName().C_Str(), path);
			}

			materials.push_back(material);
		}

		Application::s_Application->m_Window->DefaultWorkDirectory();
		DD_INFO("{} materials loaded", materials.size());

		std::vector<Mesh*> meshes;
		meshes.reserve(model->mNumMeshes);

		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			meshes.push_back(LoadMesh(model->mMeshes[i], materials[model->mMeshes[i]->mMaterialIndex]));
		}

		return new Model(meshes);
	}
}