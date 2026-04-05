#pragma once

#include "Core/Data/AssetManager.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/RenderAPI.h"

#include "Core/Math/Camera/FreeCamera.h"
#include "Core/Math/Matrix/Mat4.h"

namespace Dodo {

    class Skybox {
      private:
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<CubeMap> m_CubeMap;
        Ref<TextureSampler> m_Sampler;
        Ref<Pipeline> m_Shader;

      public:
        Skybox(std::vector<std::string> paths, AssetManager& assets, RenderAPI& renderAPI);
        ~Skybox();

        void Draw(const Math::FreeCamera& camera, RenderAPI& renderAPI) const;
    };
} // namespace Dodo
