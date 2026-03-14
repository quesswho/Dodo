#pragma once

#include "Core/Data/AssetManager.h"
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/RenderApi.h"
#include "Core/Graphics/Shader/Shader.h"

#include "Core/Math/Matrix/Mat4.h"

namespace Dodo {

    class Skybox {
      private:
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        Ref<CubeMap> m_CubeMap;
        Ref<TextureSampler> m_Sampler;
        Ref<Shader> m_Shader;

      public:
        Math::Mat4 m_Projection;
        Skybox(const Math::Mat4& projection, std::vector<std::string> paths, AssetManager& assets);
        ~Skybox();

        void Draw(const Math::Mat4& viewMatrix, RenderAPI& renderAPI) const;
    };
} // namespace Dodo