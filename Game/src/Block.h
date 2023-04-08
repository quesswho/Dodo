#pragma once

#include <Dodo.h>

class Block {
public:
	Block(Ref<Dodo::Mesh> mesh)
		: m_Mesh(mesh)
	{}
	Ref<Dodo::Mesh> m_Mesh;
};