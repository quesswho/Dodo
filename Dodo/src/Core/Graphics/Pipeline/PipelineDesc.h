#pragma once

#include "Core/Data/AssetTypes.h"
#include "Core/Graphics/RenderAPITypes.h"

namespace Dodo {
    struct PipelineDesc {
        ShaderID shaderID = 0;
        DepthComparisonMethod depthMode = DepthComparisonMethod::DEFAULT;
        bool depthTest = true;
        bool depthWrite = true;
        bool blending = false;
        bool culling = true;
        bool backfaceCull = true;
        bool stencilTest = false;
    };
} // namespace Dodo