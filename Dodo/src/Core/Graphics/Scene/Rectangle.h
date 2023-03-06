#pragma once

#include "Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

namespace Dodo {

	class Rectangle {
	private:
		Mesh* m_Mesh;
	public:
		Rectangle(Ref<Material> material);
		~Rectangle();


		template<typename T>
		inline void SetUniform(const char* location, T value) { m_Mesh->SetUniform(location, value); }

		void Draw() const;
		void Draw(Ref<Material> material) const;
	};
}