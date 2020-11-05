#pragma once
#include "Mesh.h"

namespace Dodo {
	class MeshFactory
	{
	public:
		MeshFactory();

		Mesh* GetRectangleMesh();
	private:
		const BufferProperties m_BasicProperties;
	};
}