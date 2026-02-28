#pragma once

#include <Dodo.h>

class TextureAtlas {
public:
	TextureAtlas(std::string path, uint sizex, uint sizey) 
		: m_Material()
	{}
	Dodo::Material* m_Material;


};