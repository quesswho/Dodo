#pragma once

#include <stdint.h>

namespace Dodo {

    enum class GpuTimestampSlot : uint32_t {
        Frame      = 0,
        Shadow     = 1,
        Scene      = 2,
        PostEffect = 3,
        Count      = 4,
    };

    struct GpuTimings {
        float frameMs      = 0.0f;
        float shadowMs     = 0.0f;
        float sceneMs      = 0.0f;
        float postEffectMs = 0.0f;
    };

} // namespace Dodo
