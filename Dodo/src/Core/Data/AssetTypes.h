#pragma once

#include <Core/Common.h>

namespace Dodo {
    using MaterialID = uint64_t;
    using ShaderID = uint64_t; // Spir-v or GLSL
    using PipelineID = uint64_t;
    using TextureID = uint64_t;
    using ModelID = uint64_t;

    struct TextureData {
        std::vector<uint8_t> pixels;
        int width, height, channels;
        std::string path;
    };
} // namespace Dodo
