#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLRenderAPI.h"
namespace Dodo {
	using RenderAPI = Platform::OpenGLRenderAPI; 
}
#elif DD_API_DX11

#endif
