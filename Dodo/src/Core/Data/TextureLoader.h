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
        /**
         * Loads pixel data to CPU memory from disk. Thread-safe.
         * Dispatches to LoadHDR or LoadLDR based on file content.
         * Returns TextureData with empty pixels on failure.
         */
        TextureData Load(const std::string& path);

    private:
        TextureData LoadHDR(const std::string& path);
        TextureData LoadLDR(const std::string& path);
    };

} // namespace Dodo
