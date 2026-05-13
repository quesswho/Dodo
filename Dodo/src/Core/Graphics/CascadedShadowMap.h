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

        void UpdateCamera(const Math::Mat4& proj, const Math::Mat4& view, const Math::Vec3& lightDir, float nearPlane,
                          float farPlane, float fov, float aspectRatio);
        void Bind(RenderAPI& renderAPI);

        CsmData GetCsmData() const;
        Ref<FrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }

        uint32_t m_Levels;

      private:
        std::vector<Math::Vec4> GetFrustumCornersWorldSpace(const Math::Mat4& proj, const Math::Mat4& view);

        std::vector<Math::Mat4> m_LightSpaceMatrices;
        std::vector<float> m_CascadeSplitDepths;
        Ref<FrameBuffer> m_FrameBuffer;
        uint32_t m_ShadowMapResolution;
    };
} // namespace Dodo
