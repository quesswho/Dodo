#pragma once

#include <string>

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

    enum class RenderInitStatus {
        Success,
        Failed
    };

    struct RenderInitError {
        RenderInitError(RenderInitStatus status) : status(status) {}
        RenderInitError(RenderInitStatus status, const std::string& message) : status(status), message(message) {}
        RenderInitStatus status;
        std::string message;
    };
} // namespace Dodo