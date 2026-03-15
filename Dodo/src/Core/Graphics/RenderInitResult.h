#pragma once
#include <string>

namespace Dodo {

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