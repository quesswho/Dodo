#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLBuffer.h"
namespace Dodo {
    using VertexBuffer = Platform::OpenGLVertexBuffer;
    using IndexBuffer = Platform::OpenGLIndexBuffer;
} // namespace Dodo
#elif DD_API_DX11

#endif
