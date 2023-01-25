#include "pch.h"
#include "Model.h"

namespace Dodo {

	Model::Model(std::vector<Mesh*> meshes, Material* material)
		: m_Meshes(meshes), m_Material(material)
	{}

	Model::~Model()
	{
		for(auto mesh : m_Meshes)
			delete mesh;
		delete m_Material;
	}

	void Model::Bind() const
	{ 
		m_Material->Bind(); 
	}

	void Model::Draw() const
	{
		for (auto mesh : m_Meshes)
			mesh->Draw();
	}
}