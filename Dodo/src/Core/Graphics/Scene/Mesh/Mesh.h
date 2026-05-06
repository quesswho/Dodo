#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/Material/Material.h"
#include "Core/Math/Vector/Vec3.h"

#include <vector>

namespace Dodo {

    class Mesh {
      private:
        Ref<Material> m_Material;
        Ref<VertexBuffer> m_VBuffer;
        Ref<IndexBuffer> m_IBuffer;

      public:
        Mesh(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<Material> material);
        ~Mesh();

        Ref<Material> GetMaterial() const { return m_Material; }

        void Draw(RenderAPI& renderAPI) const;
        void DrawGeometry(RenderAPI& renderAPI) const;
        void DrawGeometryInstanced(RenderAPI& renderAPI, uint instances) const;
        void Draw(Ref<Material> material, RenderAPI& renderAPI) const;
    };
} // namespace Dodo