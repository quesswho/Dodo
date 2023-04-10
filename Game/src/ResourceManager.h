#pragma once

#include <Dodo.h>

class ResourceManager {
private:

public:
	ResourceManager();

	Ref<Dodo::IndexBuffer> m_FaceIBuffer;

	Ref<Dodo::VertexBuffer> m_FrontVBuffer;
	Ref<Dodo::VertexBuffer> m_TopVBuffer;
	Ref<Dodo::VertexBuffer> m_BackVBuffer;
	Ref<Dodo::VertexBuffer> m_BottomVBuffer;
	Ref<Dodo::VertexBuffer> m_LeftVBuffer;
	Ref<Dodo::VertexBuffer> m_RightVBuffer;

	Ref<Dodo::Material> m_GrassMaterial;
};