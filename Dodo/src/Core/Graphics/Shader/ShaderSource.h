#pragma once

#include <string>
#include <vector>

namespace Dodo {
    enum class ShaderStage {
        Vertex,
        Fragment,
        Geometry,
        Compute
    };

    struct ShaderStageSource {
        ShaderStage stage;
        std::string source;
        std::string EntryPoint = "main"; // Vulkan uses entry points, so we need to specify it.
    };

    struct ShaderSource {
        std::vector<ShaderStageSource> stages;
    };
} // namespace Dodo