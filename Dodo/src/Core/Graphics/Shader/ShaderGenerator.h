#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLShaderGenerator.h"
namespace Dodo {
    using ShaderGenerator = Platform::OpenGLShaderGenerator;
}
#elif defined(DD_API_VULKAN)

#endif
