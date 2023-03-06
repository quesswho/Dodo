#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLTexture.h"
namespace Dodo { using Texture = Platform::OpenGLTexture; }
#elif DD_API_DX11

#endif
