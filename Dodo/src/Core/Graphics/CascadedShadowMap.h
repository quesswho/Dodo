#pragma once

#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/Graphics/RenderAPITypes.h"
#include "Core/Math/Matrix/Mat4.h"
#include "Core/Math/Vector/Vec3.h"

namespace Dodo {

    class CascadedShadowMap {
      public:
        CascadedShadowMap(RenderAPI& renderAPI, uint32_t levels = 4, uint32_t shadowMapResolution = 2048);
        ~CascadedShadowMap();

        /**
         * Fits the cascades to the camera frustum. Call once per frame before the shadow pass.
         *
         * @param view Camera view matrix.
         * @param lightDir Direction the directional light travels in.
         * @param fov Vertical field of view of the camera, in degrees.
         */
        void UpdateCamera(const Math::Mat4& view, const Math::Vec3& lightDir, float nearPlane, float farPlane,
                          float fov, float aspectRatio);
        void Bind(RenderAPI& renderAPI);

        CsmData GetCsmData() const { return m_CsmData; }
        Ref<FrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }

        uint32_t m_Levels;

      private:
        Ref<FrameBuffer> m_FrameBuffer;
        uint32_t m_ShadowMapResolution;

        CsmData m_CsmData;
    };
} // namespace Dodo
