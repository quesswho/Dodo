#pragma once

#include "TextureLoader.h"

#include <array>
#include <string>
#include <vector>

namespace Dodo {

    struct CubeMapData {
        std::array<TextureData, 6> faces; // faces[0].pixels.empty() on failure
    };

    struct CubeMapLoader {
        /**
         * Loads 6 cubemap face images to CPU memory from disk. Thread-safe.
         * Faces must share the same dimensions. All faces are loaded as RGBA.
         * Returns CubeMapData with empty faces[0].pixels on failure.
         * Expected face order: +X, -X, +Y, -Y, +Z, -Z.
         */
        CubeMapData Load(const std::vector<std::string>& paths);
    };

} // namespace Dodo
