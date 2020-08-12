#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLShader.h"
namespace Dodo { typedef Platform::OpenGLShader Shader; }
#elif DD_API_DX11

#endif
