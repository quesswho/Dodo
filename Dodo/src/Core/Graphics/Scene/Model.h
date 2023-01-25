#pragma once

#include "Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

namespace Dodo {

	class Model {
	private:
		Material* m_Material;
		std::vector<Mesh*> m_Meshes;
	public:
		Model(std::vector<Mesh*> meshes, Material* material);
		~Model();

		inline void SetMaterial(Material* material) { m_Material = material; }

		template<typename T>
		inline void SetUniform(const char* location, T value) { m_Material->SetUniform(location, value); }
		void Bind() const;
		void Draw() const;
	};
}