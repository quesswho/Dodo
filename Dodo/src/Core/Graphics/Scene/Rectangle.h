#pragma once

#include "Core/Graphics/Material/Material.h"
#include "Mesh/Mesh.h"

namespace Dodo {

    class Rectangle {
      private:
        Mesh *m_Mesh;

      public:
        Rectangle(Ref<Material> material);
        ~Rectangle();

        template <typename T> inline void SetUniform(const char *location, T value)
        {
            m_Mesh->SetUniform(location, value);
        }

        Mesh *GetMesh() const
        {
            return m_Mesh;
        }

        void Draw() const;
        void DrawGeometry() const;
        void Draw(Ref<Material> material) const;
    };
} // namespace Dodo