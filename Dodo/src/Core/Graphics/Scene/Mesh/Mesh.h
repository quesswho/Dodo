#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Buffer.h"
#include "Core/Math/Vector/Vec3.h"
#include "Core/Graphics/Material/Material.h"

#include <vector>

namespace Dodo {

	class Mesh {
	private:
		Ref<Material> m_Material;
		VertexBuffer* m_VBuffer;
		IndexBuffer* m_IBuffer;
	public:
		Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, Ref<Material> material);
		~Mesh();
		
		template<typename T>
		void SetUniform(const char* location, T value) { m_Material->SetUniform(location, value); }

		Ref<Material> GetMaterial() const { return m_Material; }

		void Draw() const;
		void DrawGeometry() const;
		void Draw(Ref<Material> material) const;
	};
}