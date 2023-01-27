#include "pch.h"
#include "Model.h"

namespace Dodo {

	Model::Model(std::vector<Mesh*> meshes)
		: m_Meshes(meshes)
	{}

	Model::~Model()
	{
		for(auto mesh : m_Meshes)
			delete mesh;
	}

	void Model::Draw() const
	{
		for (auto mesh : m_Meshes)
			mesh->Draw();
	}

	void Model::Draw(Material* material) const
	{
		for (auto mesh : m_Meshes)
			mesh->Draw(material);
	}
}