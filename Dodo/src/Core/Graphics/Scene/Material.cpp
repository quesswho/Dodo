#include "pch.h"
#include "Material.h"

namespace Dodo {

	Material::Material()
	{}

	Material::Material(Shader* shader, std::vector<Texture*> textures)
		: m_Shader(shader), m_Textures(textures)
	{
		std::vector<uint> index;
		index.resize(textures.size(), -1);
		for (auto& tex : m_Textures)
		{
			if (std::find(index.begin(), index.end(), tex->m_Index) != index.end())
				DD_WARN("Duplicate Texture indexes found for index: {}", tex->m_Index);
			else
				index.push_back(tex->m_Index);
		}
	}

	Material::~Material()
	{
		for (auto tex : m_Textures)
			delete tex;
	}

	void Material::AddTexture(Texture* texture)
	{
		m_Textures.push_back(texture);
		for (auto& tex : m_Textures)
		{
			if (texture->m_Index == tex->m_Index)
				DD_WARN("Duplicate Texture indexes found for index: {}", tex->m_Index);
		}
	}

	void Material::Bind()
	{
		for (auto& tex : m_Textures)
			tex->Bind();
	}
}