#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLSampler.h"
namespace Dodo {
    using TextureSampler = Platform::OpenGLSampler;
}
#elif defined(DD_API_VULKAN)

#endif
