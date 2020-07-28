#pragma once

#include "Mesh.h"
#include "Material.h"

namespace Dodo {

	struct Vertex {
		Math::Vec3 m_Position;
		Math::Vec2 m_Texcoord;
		Math::Vec3 m_Normal;
		Math::Vec3 m_Tangent;
		// Math::Vec3 m_Bitangent; // cross(m_Normal, m_Tangent)
	};

	class Model {
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint> m_Indices;

		std::vector<Material*> m_Material;
		Mesh* m_Mesh;
	public:

		// Temporary loading approach
		Model(const char* path);
		~Model();

		inline void AddMaterial(Material* material) { m_Material.push_back(material); }

		template<typename T>
		inline void SetUniform(const char* location, T value) { m_Material[0]->SetUniform(location, value); }
		void Bind() const;
		void Draw() const;
	private:
		bool Load(const char* path);
	public:
		uint m_Count;
	};
}