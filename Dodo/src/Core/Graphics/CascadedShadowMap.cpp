#include "CascadedShadowMap.h"

#include "Core/Graphics/FrameBufferProperties.h"
#include "Core/Graphics/Material/SamplerProperties.h"
#include "Core/Math/MathFunc.h"
#include "Core/Utilities/Logger.h"

#include <algorithm>
#include <cmath>

namespace Dodo {

    CascadedShadowMap::CascadedShadowMap(RenderAPI& renderAPI, uint32_t levels, uint32_t shadowMapResolution)
        : m_Levels(levels), m_ShadowMapResolution(shadowMapResolution)
    {
        if (levels != 4) {
            // Right now there is too much work required to support any number of cascades.
            // Supporting any cascades means that we would need to write slang shader specialization logic to allow us
            // to compile with We would also need to create memory with paddings for arbitrary levels in the vulkan
            // renderer
            DD_WARN("Only 4 cascades are supported, overriding levels to 4!");
            levels = 4;
        }

        FrameBufferProperties props(m_ShadowMapResolution, m_ShadowMapResolution,
                                    FrameBufferType::FRAMEBUFFER_DEPTH_ARRAY, levels);
        props.m_SamplerProperties =
            SamplerProperties(SamplerFilter::MIN_MAG_LINEAR, SamplerWrapMode::WRAP_CLAMP_TO_BORDER,
                              SamplerWrapMode::WRAP_CLAMP_TO_BORDER)
                .WithBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
        m_FrameBuffer = renderAPI.CreateFrameBuffer(props);
    }

    CascadedShadowMap::~CascadedShadowMap() {}

    void CascadedShadowMap::UpdateCamera(const Math::Mat4& view, const Math::Vec3& lightDir, float nearPlane,
                                         float farPlane, float fov, float aspectRatio)
    {
        constexpr float lambda = 0.75f; // blend between log and uniform split distributions

        // Compute cascade far-plane depths with a logarithmic-linear split scheme
        // https://dl.acm.org/doi/pdf/10.1145/1128923.1128975
        for (uint32_t i = 0; i < m_Levels; i++) {
            float p = (float)(i + 1) / (float)m_Levels;
            float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            float uniSplit = nearPlane + (farPlane - nearPlane) * p;
            m_CsmData.cascadeSplitDepths[i] = lambda * logSplit + (1.0f - lambda) * uniSplit;
        }

        const Math::Vec3 lightDirNorm = Math::Normalize(lightDir);
        // Avoid up = lightDir singularity
        const Math::Vec3 up =
            (std::abs(lightDirNorm.y) > 0.99f) ? Math::Vec3(0.0f, 0.0f, 1.0f) : Math::Vec3(0.0f, 1.0f, 0.0f);

        // The light view only rotates, it is anchored at the world origin so that it does not depend on the camera.
        // Each cascade is then a fixed-size box that slides over a world-fixed texel grid, which keeps the shadow
        // edges stable when the camera moves or rotates.
        const Math::Mat4 lightView = Math::Mat4::LookAt(Math::Vec3(0.0f, 0.0f, 0.0f) - lightDirNorm,
                                                        Math::Vec3(0.0f, 0.0f, 0.0f), up);
        const Math::Mat4 invView = Math::Mat4::Inverse(view);

        // Distance from the view axis to a frustum corner, per unit of view depth
        const float tanHalfFov = std::tan(Math::ToRadians(fov) / 2.0f);
        const float cornerSlopeSq = tanHalfFov * tanHalfFov * (1.0f + aspectRatio * aspectRatio);

        float prevSplit = nearPlane;
        for (uint32_t i = 0; i < m_Levels; i++) {
            const float splitNear = prevSplit;
            const float splitFar = m_CsmData.cascadeSplitDepths[i];
            prevSplit = splitFar;

            // Minimal bounding sphere of the sub-frustum. Its center lies on the view axis, and its radius depends
            // only on the camera parameters, so the cascade keeps its size no matter where the camera looks.
            float centerDepth;
            float radius;
            if (cornerSlopeSq >= (splitFar - splitNear) / (splitFar + splitNear)) {
                centerDepth = splitFar;
                radius = splitFar * std::sqrt(cornerSlopeSq);
            } else {
                centerDepth = 0.5f * (splitFar + splitNear) * (1.0f + cornerSlopeSq);
                const float farToCenter = splitFar - centerDepth;
                radius = std::sqrt(farToCenter * farToCenter + splitFar * splitFar * cornerSlopeSq);
            }

            const Math::Vec4 centerWS = invView * Math::Vec4(0.0f, 0.0f, -centerDepth, 1.0f);
            const Math::Vec4 centerLS = lightView * centerWS;

            // Snap the center to whole shadow map texels to eliminate sub-texel shadow crawl
            const float texelSize = 2.0f * radius / (float)m_ShadowMapResolution;
            const float centerX = std::floor(centerLS.x / texelSize) * texelSize;
            const float centerY = std::floor(centerLS.y / texelSize) * texelSize;

            // The light looks down its negative Z axis, so shadow casters between the light and the sphere have a
            // larger Z. Pull the near plane back to capture them, anything even closer is depth clamped.
            constexpr float casterRangeScale = 6.0f;
            const float zNear = -(centerLS.z + radius + casterRangeScale * radius);
            const float zFar = -(centerLS.z - radius);

            Math::Mat4 lightProj = Math::Mat4::Orthographic(centerX - radius, centerX + radius, centerY - radius,
                                                            centerY + radius, zNear, zFar);
            m_CsmData.lightSpaceMatrices[i] = lightProj * lightView;
        }
        m_CsmData.numCascades = (int)m_Levels;
        m_CsmData.pad[0] = m_CsmData.pad[1] = m_CsmData.pad[2] = 0.0f;
    }

    void CascadedShadowMap::Bind(RenderAPI& renderAPI)
    {
        renderAPI.BindFrameBuffer(m_FrameBuffer);
    }
} // namespace Dodo
