#pragma once

#include "Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

namespace Dodo {

	class Model {
	private:
		std::vector<Mesh*> m_Meshes;
	public:
		Model(std::vector<Mesh*> meshes);
		~Model();

		template<typename T>
		void SetUniform(const char* location, T value) { for(auto mesh : m_Meshes) mesh->SetUniform(location, value); }

		void Draw() const;
		void Draw(Material* material) const;
	};
}