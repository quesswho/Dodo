#include "CascadedShadowMap.h"

#include "Core/Graphics/FrameBufferProperties.h"
#include "Core/Graphics/Material/SamplerProperties.h"
#include "Core/Math/MathFunc.h"

#include <algorithm>
#include <cmath>

namespace Dodo {

    CascadedShadowMap::CascadedShadowMap(RenderAPI& renderAPI, uint32_t levels) : m_Levels(levels)
    {
        FrameBufferProperties props(4096, 4096, FrameBufferType::FRAMEBUFFER_DEPTH_ARRAY, levels);
        props.m_SamplerProperties =
            SamplerProperties(SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_BORDER,
                              SamplerWrapMode::WRAP_CLAMP_TO_BORDER)
                .WithBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
        m_FrameBuffer = renderAPI.CreateFrameBuffer(props);
    }

    CascadedShadowMap::~CascadedShadowMap() {}

    std::vector<Math::Vec4> CascadedShadowMap::GetFrustumCornersWorldSpace(const Math::Mat4& proj,
                                                                           const Math::Mat4& view)
    {
        const Math::Mat4 inv = Math::Mat4::Inverse(proj * view);

        std::vector<Math::Vec4> frustumCorners;
        frustumCorners.reserve(8);
        for (uint32_t x = 0; x < 2; x++) {
            for (uint32_t y = 0; y < 2; y++) {
                for (uint32_t z = 0; z < 2; z++) {
                    const Math::Vec4 pt = inv * Math::Vec4(x * 2.0f - 1.0f, y * 2.0f - 1.0f, z * 2.0f - 1.0f, 1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }
        return frustumCorners;
    }

    void CascadedShadowMap::UpdateCamera(const Math::Mat4& proj, const Math::Mat4& view, const Math::Vec3& lightDir,
                                         float nearPlane, float farPlane, float fov, float aspectRatio)
    {
        constexpr float lambda = 0.5f; // blend between log and uniform split distributions

        // Compute cascade far-plane depths with a logarithmic-linear blend
        float cascadeSplits[4];
        for (uint32_t i = 0; i < m_Levels; i++) {
            float p = (float)(i + 1) / (float)m_Levels;
            float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            float uniSplit = nearPlane + (farPlane - nearPlane) * p;
            cascadeSplits[i] = lambda * logSplit + (1.0f - lambda) * uniSplit;
        }
        // Store as Vec4 components for the shader
        m_CascadeSplitDepths = Math::Vec4(cascadeSplits[0], cascadeSplits[1], cascadeSplits[2], cascadeSplits[3]);

        const Math::Vec3 lightDirNorm = Math::Normalize(lightDir);
        // Avoid up = lightDir singularity
        const Math::Vec3 up =
            (std::abs(lightDirNorm.y) > 0.99f) ? Math::Vec3(0.0f, 0.0f, 1.0f) : Math::Vec3(0.0f, 1.0f, 0.0f);

        float prevSplit = nearPlane;
        for (uint32_t i = 0; i < m_Levels; i++) {
            float splitFar = cascadeSplits[i];

            // Build a perspective matrix for just this sub-frustum
            Math::Mat4 subProj = Math::Mat4::Perspective(fov, aspectRatio, prevSplit, splitFar);
            prevSplit = splitFar;

            // Get the 8 world-space corners of this sub-frustum
            std::vector<Math::Vec4> corners = GetFrustumCornersWorldSpace(subProj, view);

            // Centroid of the 8 corners is the anchor for the light view matrix
            Math::Vec3 centroid(0.0f, 0.0f, 0.0f);
            for (const auto& c : corners)
                centroid = centroid + Math::Vec3(c.x, c.y, c.z);
            centroid = centroid * (1.0f / 8.0f);

            Math::Mat4 lightView = Math::Mat4::LookAt(centroid - lightDirNorm, centroid, up);

            // Find a tight AABB of all corners in light space
            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();

            for (const auto& c : corners) {
                Math::Vec4 inLight = lightView * c;
                minX = std::min(minX, inLight.x);
                maxX = std::max(maxX, inLight.x);
                minY = std::min(minY, inLight.y);
                maxY = std::max(maxY, inLight.y);
                minZ = std::min(minZ, inLight.z);
                maxZ = std::max(maxZ, inLight.z);
            }

            // Pull the near plane back to capture shadow casters behind the frustum
            constexpr float zMult = 10.0f;
            if (minZ < 0.0f)
                minZ *= zMult;
            else
                minZ /= zMult;
            if (maxZ < 0.0f)
                maxZ /= zMult;
            else
                maxZ *= zMult;

            Math::Mat4 lightProj = Math::Mat4::Orthographic(minX, maxX, minY, maxY, minZ, maxZ);
            m_LightSpaceMatrices[i] = lightProj * lightView;
        }
    }

    void CascadedShadowMap::Bind(RenderAPI& renderAPI)
    {
        renderAPI.BindFrameBuffer(m_FrameBuffer);
    }

    CsmData CascadedShadowMap::GetCsmData() const
    {
        CsmData data;
        for (uint32_t i = 0; i < m_Levels; i++)
            data.lightSpaceMatrices[i] = m_LightSpaceMatrices[i];
        data.cascadeSplitDepths = m_CascadeSplitDepths;
        data.numCascades = (int)m_Levels;
        data.pad[0] = data.pad[1] = data.pad[2] = 0.0f;
        return data;
    }
} // namespace Dodo
