#pragma once

#include <string>
#include <vector>

namespace Dodo {
    enum class ShaderStage {
        Unknown,
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
        std::string name;
        std::vector<ShaderStageSource> stages;
    };
} // namespace Dodo