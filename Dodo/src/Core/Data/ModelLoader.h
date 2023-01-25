#pragma once

#include "Core/Graphics/Material/ShaderBuilder.h"
#include "Core/Graphics/Scene/Model.h"

#include <assimp/scene.h>

namespace Dodo {

	struct ModelLoader {

		struct Vertex {
			Math::Vec3 m_Position;
			Math::Vec2 m_Texcoord;
			Math::Vec3 m_Normal;
			Math::Vec3 m_Tangent;
		};

		struct MeshData {
			MeshData(const std::vector<Vertex> vertices, const std::vector<uint> indices)
				: m_Vertices(vertices), m_Indices(indices)
			{}

			std::vector<Vertex> m_Vertices;
			std::vector<uint> m_Indices;
		};

		struct MaterialData {
			MaterialData(std::unordered_map<const char*, uint> textureInfo, ShaderBuilderFlags flags)
				: m_TextureInfo(textureInfo), m_Flags(flags)
			{}
			std::unordered_map<const char*, uint> m_TextureInfo;
			ShaderBuilderFlags m_Flags;
		};

		struct ModelData {
			ModelData(std::vector<Vertex> vertices, std::vector<uint> indices, std::unordered_map<const char*, uint> textureInfo, ShaderBuilderFlags flags)
				: m_MeshData(new MeshData(vertices, indices)), m_MaterialData(new MaterialData(textureInfo, flags))
			{}

			ModelData(MeshData* meshData, MaterialData* materialData)
				: m_MeshData(meshData), m_MaterialData(materialData)
			{}

			~ModelData()
			{
				delete m_MeshData;
				delete m_MaterialData;
			}
			MeshData* m_MeshData;
			MaterialData* m_MaterialData;

		};

		///////////////////////////////////////////////////

		Model* LoadModel(const char* path);
	private:
		Mesh* LoadMesh(aiMesh* mesh, Material* material);
	};
}