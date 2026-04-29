#pragma once

#include "Core/Data/AssetManager.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/MaterialSet.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/RenderAPI.h"

#include "Core/Math/Camera/FreeCamera.h"
#include "Core/Math/Matrix/Mat4.h"

namespace Dodo {

    class Skybox {
      private:
        Ref<VertexBuffer> m_VertexBuffer;
        CubeMapID m_CubeMapID = 0;
        Ref<TextureSampler> m_Sampler;
        Ref<Pipeline> m_Shader;
        AssetManager& m_Assets;
        mutable MaterialSet m_MaterialSet;

      public:
        Skybox(std::vector<std::string> paths, AssetManager& assets, RenderAPI& renderAPI);
        ~Skybox();

        void Draw(RenderAPI& renderAPI) const;
    };
} // namespace Dodo
