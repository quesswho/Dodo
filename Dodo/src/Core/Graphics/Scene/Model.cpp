#include "pch.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Core/Application/Application.h"

namespace Dodo {

	Model::Model(const char* path)
	{
		if (!Load(path))
			DD_ERR("Could not load model: {}", path);
	}

	Model::~Model()
	{
		
	}

	bool Model::Load(const char* path)
	{
		Assimp::Importer imp;

		const aiScene* model = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!model || model->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode)
			return false;
		if (model->mNumMeshes > 1)
			DD_WARN("More than one mesh loaded in one model, Merging!");
		for (uint i = 0; i < model->mNumMeshes; i++)
		{
			aiMesh* mesh = model->mMeshes[i];
			for (uint j = 0; j < mesh->mNumVertices; j++)
			{
				Vertex vertex;
				vertex.m_Position.x = mesh->mVertices[j].x;
				vertex.m_Position.y = mesh->mVertices[j].y;
				vertex.m_Position.z = mesh->mVertices[j].z;
				if (mesh->mTextureCoords[0])
				{
					vertex.m_Texcoord.x = mesh->mTextureCoords[0][j].x;
					vertex.m_Texcoord.y = mesh->mTextureCoords[0][j].y;
				}
				else
				{
					vertex.m_Texcoord.x = 0.0f;
					vertex.m_Texcoord.y = 0.0f;
				}

				vertex.m_Normal.x = mesh->mNormals[j].x;
				vertex.m_Normal.y = mesh->mNormals[j].y;
				vertex.m_Normal.z = mesh->mNormals[j].z;
				vertex.m_Tangent.x = mesh->mTangents[j].x;
				vertex.m_Tangent.y = mesh->mTangents[j].y;
				vertex.m_Tangent.z = mesh->mTangents[j].z;

				m_Vertices.push_back(vertex);
			}

			for (uint k = 0; k < mesh->mNumFaces; k++)
			{
				aiFace face = mesh->mFaces[k];
				for (uint j = 0; j < face.mNumIndices; j++)
					m_Indices.push_back(face.mIndices[j]);
			}
		}
		m_Count = (uint)m_Indices.size();
		m_Mesh = new Mesh(new VertexBuffer((float*)&m_Vertices[0], (uint)m_Vertices.size() * sizeof(Vertex), BufferProperties({ { "POSITION", 3 }, { "TEXCOORD", 2 }, { "NORMAL", 3 }, { "TANGENT", 3 } })), new IndexBuffer(m_Indices.data(), (uint)m_Indices.size()));
		ShaderBuilderFlags flags = ShaderBuilderFlagNone;
		for (uint i = 0; i < model->mNumMaterials; i++)
		{
			aiMaterial* mat = model->mMaterials[i];
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
			str = "";
			Material* material;
			if (textures.size() > 0)
				material = new Material(Application::s_Application->m_AssetManager->GetShader(flags), textures);
				//material = new Material(Application::s_Application->m_RenderAPI->m_ShaderBuilder->BuildVertexFragmentShader(path, flags), textures);
			else
				material = new Material(); // Fallback shader

			m_Material.push_back(material);
		}

		if (!model->HasMaterials())
		{
			m_Material.push_back(new Material());
		}
		return true;
	}

	void Model::Bind() const
	{ 
		m_Material[0]->Bind(); 
	}

	void Model::Draw() const
	{
		m_Mesh->Draw();
		Application::s_Application->m_RenderAPI->DrawIndices(m_Count);
	}
}