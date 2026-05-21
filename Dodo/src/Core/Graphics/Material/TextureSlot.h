#pragma once

#include "Core/Common.h"

namespace Dodo {

    enum class TextureSlot : uint {
        Albedo    = 0,
        Roughness = 1,
        Normal    = 2,
        Metallic  = 3,
        Ao        = 4,
        Spec      = 5,
        Count     = 6,
    };

}
