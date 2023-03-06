#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLFrameBuffer.h"
namespace Dodo { using FrameBuffer = Platform::OpenGLFrameBuffer; }
#elif DD_API_DX11

#endif
