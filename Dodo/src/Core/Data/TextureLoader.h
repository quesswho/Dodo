#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo {

    struct TextureData {
        std::vector<uchar> pixels; // Empty on failure
        TextureProperties props;
    };

    struct TextureLoader {
        // Loads pixel data from disk (CPU only, no GPU calls).
        // Returns TextureData with empty pixels on failure.
        TextureData Load(const std::string& path);
    };

} // namespace Dodo
