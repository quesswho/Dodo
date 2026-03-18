#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLCubeMap.h"
namespace Dodo {
    using CubeMap = Platform::OpenGLCubeMap;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanCubeMap.h"
namespace Dodo {
    using CubeMap = Platform::VulkanCubeMap;
}
#endif
