#include "pch.h"
#include "Model.h"

#include "Core/Application/Application.h"

namespace Dodo {

	Model::Model(const Mesh* mesh, Material* material)
		: m_Mesh(mesh), m_Material(material)
	{}

	Model::~Model()
	{
		delete m_Mesh;
		delete m_Material;
	}

	void Model::Bind() const
	{ 
		m_Material->Bind(); 
	}

	void Model::Draw() const
	{
		m_Mesh->Draw();
	}
}