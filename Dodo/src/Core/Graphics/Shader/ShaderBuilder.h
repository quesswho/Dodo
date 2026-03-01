#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLShaderBuilder.h"
namespace Dodo {
    using ShaderBuilder = Platform::OpenGLShaderBuilder;
}
#elif DD_API_DX11

#endif
