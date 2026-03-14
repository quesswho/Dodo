#pragma once

namespace Dodo {
    enum class DepthComparisonMethod {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
        DEFAULT = LESS
    };
} // namespace Dodo