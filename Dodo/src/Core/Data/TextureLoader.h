#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

#include <string>
#include <vector>

namespace Dodo {

    struct TextureData {
        std::vector<uchar> pixels;       // Empty on failure
        std::vector<size_t> mipOffsets;  // Byte offset per mip into pixels (non-empty only for DDS/Preloaded)
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
        TextureData LoadDDS(const std::string& path);

        /**
         * Returns 4 channels if the image has 3 or 4 channels, otherwise returns the original channel count.
         * This ensures the loader pads to RGBA for optimal memory tiling when needed
         */
        int GetDesiredChannels(const std::string& path);
    };

} // namespace Dodo
