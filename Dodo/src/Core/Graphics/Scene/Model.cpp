#include "Model.h"
#include "pch.h"

namespace Dodo {

    Model::Model(std::vector<Mesh *> meshes) : m_Meshes(meshes)
    {}

    Model::~Model()
    {
        for (auto mesh : m_Meshes)
            delete mesh;
    }

    void Model::Draw() const
    {
        for (auto mesh : m_Meshes)
            mesh->Draw();
    }

    void Model::DrawGeometry() const
    {
        for (auto mesh : m_Meshes)
            mesh->DrawGeometry();
    }

    void Model::Draw(Ref<Material> material) const
    {
        material->Bind();
        for (auto mesh : m_Meshes)
            mesh->DrawGeometry();
    }
} // namespace Dodo