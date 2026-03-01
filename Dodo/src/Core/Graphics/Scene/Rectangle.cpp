#include "Rectangle.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Rectangle::Rectangle(Ref<Material> material)
        : m_Mesh(Application::s_Application->m_AssetManager->m_MeshFactory->GetRectangleMesh(material))
    {}

    Rectangle::~Rectangle() {}

    void Rectangle::Draw() const { m_Mesh->Draw(); }

    void Rectangle::DrawGeometry() const { m_Mesh->DrawGeometry(); }

    void Rectangle::Draw(Ref<Material> material) const { m_Mesh->Draw(material); }
} // namespace Dodo