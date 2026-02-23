#pragma once

#ifdef DD_API_WIN32
#include "Platform/WindowAPI/Win32/Win32Window.h"
namespace Dodo { using Window = Platform::Win32Window; }
#elif DD_API_GLFW
#include "Platform/WindowAPI/GLFW/GLFWWindow.h"
namespace Dodo { using Window = Platform::GLFWWindow; }
#else
#error No Window API selected!
#endif
