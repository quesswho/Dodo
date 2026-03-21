#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Dodo {
    struct ShaderAsset {
        std::vector<uint32_t> spirv;
        std::string glsl;
        std::string path; // source path for hot reload
    };
} // namespace Dodo