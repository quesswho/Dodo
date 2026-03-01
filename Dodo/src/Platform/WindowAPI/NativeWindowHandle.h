#pragma once

namespace Dodo {

    struct NativeWindowHandle {
        enum class WindowBackend
        {
            Win32,
            GLFW
        };

        WindowBackend backend;
        void *window;  // HWND or GLFWHandle
        void *display; // HDC
    };
} // namespace Dodo