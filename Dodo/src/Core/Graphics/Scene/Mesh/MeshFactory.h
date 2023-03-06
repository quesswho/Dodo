#pragma once

#include "Mesh.h"

namespace Dodo {
	class MeshFactory
	{
	public:
		MeshFactory();

		Mesh* GetRectangleMesh(Ref<Material> material);
	private:
		const BufferProperties m_BasicProperties;
		Mesh* m_RectangleMesh;
	};
}