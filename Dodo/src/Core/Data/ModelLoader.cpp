#include "pch.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Core/Application/Application.h"

namespace Dodo {

	Model* ModelLoader::LoadModel(const char* path)
	{
		Assimp::Importer imp;

		const aiScene* model = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!model || model->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode)
		{
			DD_WARN("Unable to load model: {}", path);
			return nullptr; // TODO: Replace with error model
		}
		if (model->mNumMeshes > 1)
			DD_WARN("More than one mesh loaded in one model, Merging!");
		uint totalVertices = 0;
		uint totalIndices = 0;
		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			totalVertices += model->mMeshes[i]->mNumVertices;
			totalIndices += model->mMeshes[i]->mNumFaces * 3;
		}
		std::vector<uint> indices;
		std::vector<Vertex> vertices;
		indices.reserve(totalIndices);
		vertices.reserve(totalVertices);

		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			aiMesh* mesh = model->mMeshes[i];
			for (uint j = 0; j < mesh->mNumVertices; j++)
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

			for (uint k = 0; k < mesh->mNumFaces; k++)
			{
				aiFace face = mesh->mFaces[k];
				for (uint j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
		}
		std::string workdir = path;
		if (workdir.find('/') != std::string::npos)
			workdir.erase(workdir.begin() + workdir.find_last_of('/'), workdir.end());

		Application::s_Application->m_Window->ChangeWorkDirectory(workdir);
		const Mesh* mesh = new Mesh(new VertexBuffer((float*)&vertices[0], totalVertices * sizeof(Vertex), BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3 }, { "TANGENT", 3 } })), new IndexBuffer(indices.data(), totalIndices));
		Material* material = nullptr;
		if (!model->HasMaterials())
		{
			material = new Material();
		}
		else
		{
			if(model->mNumMaterials > 1)
				DD_WARN("More than one material define, using the first one!");

			ShaderBuilderFlags flags = ShaderBuilderFlagNone;

			aiMaterial* mat = model->mMaterials[0];
			std::vector<Texture*> textures;
			aiString str;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagDiffuseMap;
				textures.push_back(new Texture(str.C_Str(), 0));
			}
			str = "";
			mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagSpecularMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
			}
			str = "";
			mat->GetTexture(aiTextureType_NORMALS, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagNormalMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
			}
			
			if (textures.size() > 0)
				material = new Material(Application::s_Application->m_AssetManager->GetShader(flags), textures);
			else
				material = new Material(); // Fallback shader
		}

		return new Model(mesh, material);
	}

	std::tuple<ModelLoader::ModelData*, Model*> ModelLoader::LoadModelData(const char* path)
	{
		Assimp::Importer imp;

		const aiScene* model = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!model || model->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode)
		{
			DD_WARN("Unable to load model: {}", path);
			return std::make_tuple(nullptr, nullptr); // TODO: Replace with error model
		}

		if (model->mNumMeshes > 1)
			DD_WARN("More than one mesh loaded in one model, Merging!");
		uint totalVertices = 0;
		uint totalIndices = 0;
		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			totalVertices += model->mMeshes[i]->mNumVertices;
			totalIndices += model->mMeshes[i]->mNumFaces * 3;
		}
		std::vector<uint> indices;
		std::vector<Vertex> vertices;
		indices.reserve(totalIndices);
		vertices.reserve(totalVertices);

		ShaderBuilderFlags flags = ShaderBuilderFlagNone;
		std::unordered_map<const char*, uint> textureInfo;

		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			aiMesh* mesh = model->mMeshes[i];
			for (uint j = 0; j < mesh->mNumVertices; j++)
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

			for (uint k = 0; k < mesh->mNumFaces; k++)
			{
				aiFace face = mesh->mFaces[k];
				for (uint j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
		}
		const Mesh* mesh = new Mesh(new VertexBuffer((float*)&vertices[0], totalVertices * sizeof(Vertex), BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3 }, { "TANGENT", 3 } })), new IndexBuffer(indices.data(), totalIndices));
		Material* material = nullptr;
		if (!model->HasMaterials())
		{
			material = new Material();
		}
		else
		{
			if (model->mNumMaterials > 1)
				DD_WARN("More than one material define, using the first one!");


			aiMaterial* mat = model->mMaterials[0];
			std::vector<Texture*> textures;
			aiString str;

			mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagDiffuseMap;
				textures.push_back(new Texture(str.C_Str(), 0));
				textureInfo.emplace(str.C_Str(), 0);
			}
			str = "";
			mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagSpecularMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
				textureInfo.emplace(str.C_Str(), (uint)textures.size());
			}
			str = "";
			mat->GetTexture(aiTextureType_NORMALS, 0, &str);
			if (str.length > 0)
			{
				flags |= ShaderBuilderFlagNormalMap;
				textures.push_back(new Texture(str.C_Str(), (uint)textures.size()));
				textureInfo.emplace(str.C_Str(), (uint)textures.size());
			}
			str = "";
			if (textures.size() > 0)
				material = new Material(Application::s_Application->m_AssetManager->GetShader(flags), textures);
			else
				material = new Material(); // Fallback shader
		}

		return std::make_tuple(new ModelLoader::ModelData(vertices, indices, textureInfo, flags), new Model(mesh, material));
	}
}