#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLFrameBuffer.h"
namespace Dodo { typedef Platform::OpenGLFrameBuffer FrameBuffer; }
#elif DD_API_DX11

#endif
