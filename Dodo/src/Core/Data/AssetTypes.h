#pragma once

#include <Core/Common.h>

namespace Dodo {
    using MaterialID = uint64_t;
    using ShaderID = uint64_t; // Spir-v or GLSL
    using PipelineID = uint64_t;
    using TextureID = uint64_t;
    using ModelID = uint64_t;

    enum class AssetState {
        NotLoaded, // ID allocated but loading hasn't started
        Loading,   // Currently loading on worker thread
        Staging,   // Loaded, waiting to be uploaded to GPU
        Loaded,    // Fully loaded and ready to use
        Failed     // Loading failed
    };
} // namespace Dodo
