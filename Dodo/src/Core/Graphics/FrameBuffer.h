#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLFrameBuffer.h"
namespace Dodo {
    using FrameBuffer = Platform::OpenGLFrameBuffer;
}
#elif defined(DD_API_VULKAN)

#endif
