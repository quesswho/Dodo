#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLCubeMap.h"
namespace Dodo {
    using CubeMap = Platform::OpenGLCubeMap;
}
#elif defined(DD_API_VULKAN)

#endif
