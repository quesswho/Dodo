#pragma once

#include <Core/Common.h>

#include <Core/Data/AssetManager.h>
#include <Core/Graphics/CascadedShadowMap.h>
#include <Core/Graphics/PostEffect.h>
#include <Core/Graphics/Scene/Scene.h>
#include <Core/Graphics/Skybox.h>

#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {
    /**
     * Parameters of the builtin Gamma.slang post effect.
     */
    struct GammaEffectData {
        float gamma = 1.0f;
        float exposure = 1.0f;
    };

    // Make this a deferred renderer
    class Renderer3D {
        PostEffect* m_PostEffect;

        CascadedShadowMap* m_CascadedShadowMap;
        Ref<Material> m_ShadowMapMaterial;
        Ref<Material> m_ShadowAlphaTestMaterial;

      public:
        /**
         * @param output      Framebuffer the finished image is drawn into, or nullptr for the swapchain.
         * @param msaaSamples MSAA sample count of the scene pass, 1 disables antialiasing.
         */
        Renderer3D(RenderAPI& renderAPI, AssetManager& assets, Ref<FrameBuffer> output = nullptr,
                   uint32_t msaaSamples = 4);

        ~Renderer3D() {}

        void DrawScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderApi, AssetManager& assets);
        void DrawShadowedScene(Scene* scene, const Math::FreeCamera& camera, RenderAPI& renderApi,
                               AssetManager& assets);

        void RenderEntities(World& world, const Math::FreeCamera& camera, LightSystem& lightSystem,
                            RenderAPI& renderApi, AssetManager& assets, Ref<CubeMap> irradianceMap = nullptr);
        void RenderGeometry(World& world, RenderAPI& renderApi, AssetManager& assets);

        void SetPostEffect(PostEffect* fx) { m_PostEffect = fx; }

        /**
         * Sets the tonemapping parameters of the post effect.
         */
        void SetEffectData(const GammaEffectData& data) { m_PostEffect->SetEffectData(data); }

        /**
         * Resizes the framebuffer the scene is rendered into. Call when the size of the output changes.
         */
        void Resize(uint width, uint height) { m_PostEffect->Resize(width, height); }

      private:
        static DrawData MakeDrawData(const Math::Mat4& model);
    };
} // namespace Dodo
