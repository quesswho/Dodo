#include "CascadedShadowMap.h"

namespace Dodo {

    CascadedShadowMap::CascadedShadowMap(RenderAPI& renderAPI, uint32_t layers) : m_Layers(layers) {}

    CascadedShadowMap::~CascadedShadowMap() {}

    void CascadedShadowMap::UpdateCamera(Math::Mat4 projection, Math::Mat4 view) {}

    void CascadedShadowMap::Bind(RenderAPI& renderAPI)
    {
        renderAPI.BindFrameBuffer(m_FrameBuffer);
    }

    std::vector<Math::Vec4> GetFrustumCornersWorldSpace(const Math::Mat4& proj, const Math::Mat4& view)
    {
        const Math::Mat4 inv = Math::Mat4::Inverse(proj * view);

        std::vector<Math::Vec4> frustumCorners;
        for (uint32_t x = 0; x < 2; x++) {
            for (uint32_t y = 0; y < 2; y++) {
                for (uint32_t z = 0; z < 2; z++) {
                    const Math::Vec4 pt = inv * Math::Vec4(x * 2.0f - 1.0f, y * 2.0f - 1.0f, z * 2.0f - 1.0f, 1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }
    }
} // namespace Dodo