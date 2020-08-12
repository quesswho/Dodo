#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLShaderBuilder.h"
namespace Dodo { typedef Platform::OpenGLShaderBuilder ShaderBuilder; }
#elif DD_API_DX11

#endif
