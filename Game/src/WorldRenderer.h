#pragma once

#include <Dodo.h>

#include "World.h"

class WorldRenderer {
private:
	Dodo::Math::FreeCamera* m_Camera;
	Ref<Dodo::VertexBuffer> m_CubeVBuffer;
	Ref<Dodo::IndexBuffer> m_CubeIBuffer;
	Ref<Dodo::Material> m_GrassMaterial;

public:
	WorldRenderer(Dodo::Math::FreeCamera* camera);

	void Draw(World* world);
};