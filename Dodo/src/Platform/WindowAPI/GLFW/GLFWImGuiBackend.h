#pragma once

struct GLFWwindow;

namespace Dodo::Platform {

    class GLFWImGuiBackend {
    public:
        static void Init(GLFWwindow* window, bool enableDocking = false);
        static void Shutdown();
        static void NewFrame();
        static void EndFrame();
    };
}