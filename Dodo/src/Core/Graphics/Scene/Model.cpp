#include "Model.h"
#include "pch.h"

namespace Dodo {

    Model::Model(std::vector<Mesh*> meshes) : m_Meshes(meshes) {}

    Model::~Model()
    {
        for (auto mesh : m_Meshes)
            delete mesh;
    }

    void Model::Draw(RenderAPI& renderAPI) const
    {
        for (auto mesh : m_Meshes)
            mesh->Draw(renderAPI);
    }

    void Model::DrawGeometry(RenderAPI& renderAPI) const
    {
        for (auto mesh : m_Meshes)
            mesh->DrawGeometry(renderAPI);
    }

    void Model::Draw(Ref<Material> material, RenderAPI& renderAPI) const
    {
        material->Bind(renderAPI);
        for (auto mesh : m_Meshes)
            mesh->DrawGeometry(renderAPI);
    }
} // namespace Dodo