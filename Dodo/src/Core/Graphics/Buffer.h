#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLBuffer.h"
namespace Dodo { 
	typedef Platform::OpenGLVertexBuffer VertexBuffer;
	typedef Platform::OpenGLIndexBuffer IndexBuffer;
}
#elif DD_API_DX11

#endif
