#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLShader.h"
namespace Dodo {
    using Shader = Platform::OpenGLShader;
}
#elif DD_API_DX11

#endif
