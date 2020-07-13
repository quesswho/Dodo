#pragma once

#include "Core/Common.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Math/Vector/Vec3.h"
#include <vector>

namespace Dodo {

	class Mesh {
	private:
		VertexBuffer* m_VBuffer;
		IndexBuffer* m_IBuffer;
	public:
		Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer);
		~Mesh();

		void Draw() const;
	};
}