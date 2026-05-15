#pragma once

#include <Core/Common.h>

#include <Core/Data/AssetManager.h>
#include <Core/Graphics/CascadedShadowMap.h>
#include <Core/Graphics/PostEffect.h>
#include <Core/Graphics/Scene/Scene.h>
#include <Core/Graphics/Skybox.h>

#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {
    // Make this a deferred renderer
    class Renderer3D {
        PostEffect* m_PostEffect;

        CascadedShadowMap* m_CascadedShadowMap;
        Ref<Material> m_ShadowMapMaterial;

      public:
        Renderer3D(RenderAPI& renderAPI, AssetManager& assets);

        ~Renderer3D() {}

        void DrawScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderApi, AssetManager& assets);
        void DrawShadowedScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderApi,
                               AssetManager& assets);

        void RenderEntities(World& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                            RenderAPI& renderApi, AssetManager& assets, Ref<CubeMap> irradianceMap = nullptr);
        void RenderGeometry(World& world, RenderAPI& renderApi, AssetManager& assets);

        void SetPostEffect(PostEffect* fx) { m_PostEffect = fx; }

      private:
        static DrawData MakeDrawData(const Math::Mat4& model);
    };
} // namespace Dodo
