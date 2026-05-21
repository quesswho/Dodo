#pragma once

#include "Core/Data/AssetTypes.h"
#include "Core/Graphics/BufferLayout.h"
#include "Core/Graphics/FrameBufferProperties.h"
#include "Core/Graphics/RenderAPITypes.h"

namespace Dodo {

    enum class BlendMode : uint8_t { // TODO: Add more blend modes supported by vulkan
        None,
        Opaque,
        AlphaBlend,
        AlphaCutout, // alpha test via discard, no GPU blending, depth write on
        Additive,
        Multiply
    };

    enum class DepthMode : uint8_t {
        None,
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class CullMode : uint8_t {
        None,
        Back,
        Front
    };

    struct PipelineDesc {
        ShaderID shaderID = 0;

        // Render state
        BlendMode blendMode = BlendMode::Opaque;
        DepthMode depthMode = DepthMode::Less;
        CullMode culling = CullMode::Back;
        bool depthWrite = true; // Set false for alpha-blended geometry: reads depth but does not write it
        bool stencilTest = false;
        bool depthOnly = false; // No color attachment (e.g. shadow pass)
        bool renderToSwapchain =
            false;              // Outputs directly to the swapchain; most pipelines render to the HDR offscreen buffer
        bool depthClip = false; // Anything in front of the near plane will be clamped to Z=0
        FrameBufferColorFormat colorFormat = FrameBufferColorFormat::RGBA16F;

        // When non-empty (m_Stride > 0), overrides stride and per-attribute offsets derived from
        // shader reflection. Location N maps to m_Elements[N]. Required when the shader uses a
        // subset of attributes (e.g. shadow pass uses only position from a full mesh VBO).
        BufferProperties vertexLayout;

        // When true (default), Set 1 is the global bindless texture heap shared across all pipelines.
        // Set to false for GPU compute passes (equirect, convolution) that build their own Set 1 layout
        // from shader reflection and manage their own per-pass descriptor sets.
        bool useBindlessHeap = true;
    };
} // namespace Dodo