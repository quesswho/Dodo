#pragma once

namespace Dodo {

    enum class RenderInitStatus {
        Success,
        Failed
    };

    struct RenderInitError {
        RenderInitStatus status;
        std::string message;
    };
}