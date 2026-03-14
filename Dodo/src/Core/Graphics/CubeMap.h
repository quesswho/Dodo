#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLCubeMap.h"
namespace Dodo {
    using CubeMapTexture = Platform::OpenGLCubeMapTexture;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VkCubeMap.h"
namespace Dodo {
    using CubeMapTexture = Platform::VkCubeMapTexture;
}
#endif
