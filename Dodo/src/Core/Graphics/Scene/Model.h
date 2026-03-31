#pragma once

#include "Core/Graphics/Material/Material.h"
#include "Mesh/Mesh.h"

namespace Dodo {

    class Model {
      private:
        std::vector<Ref<Mesh>> m_Meshes;
        
      public:
        Model(std::vector<Ref<Mesh>> meshes);
        ~Model();

        const std::vector<Ref<Mesh>>& GetMeshes() const { return m_Meshes; }
        void Draw(RenderAPI& renderAPI) const;
        void DrawGeometry(RenderAPI& renderAPI) const;
        void Draw(Ref<Material> material, RenderAPI& renderAPI) const;
    };
} // namespace Dodo