#pragma once

#include "Core/Data/AssetTypes.h"
#include "Core/Graphics/RenderAPITypes.h"

namespace Dodo {

    enum class BlendMode : uint8_t {
        None,
        Opaque,
        AlphaBlend,
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
        bool stencilTest = false;
    };
} // namespace Dodo