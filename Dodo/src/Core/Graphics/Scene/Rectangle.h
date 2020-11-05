#pragma once

#include "Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

namespace Dodo {

	class Rectangle {
	private:
		Material* m_Material;
		const Mesh* m_Mesh;
	public:
		Rectangle(Material* material);
		~Rectangle();

		inline void SetMaterial(Material* material) { m_Material = material; }

		template<typename T>
		inline void SetUniform(const char* location, T value) { m_Material->SetUniform(location, value); }
		void Bind() const;
		void Draw() const;
	};
}