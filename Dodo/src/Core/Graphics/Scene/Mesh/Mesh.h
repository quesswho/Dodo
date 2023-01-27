#pragma once

#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Math/Vector/Vec3.h"
#include "Core/Graphics/Material/Material.h"

#include <vector>

namespace Dodo {

	class Mesh {
	private:
		Material* m_Material;
		VertexBuffer* m_VBuffer;
		IndexBuffer* m_IBuffer;
	public:
		Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, Material* material);
		~Mesh();
		
		template<typename T>
		void SetUniform(const char* location, T value) { m_Material->SetUniform(location, value); }

		void Draw() const;
		void Draw(Material* material) const;
	};
}