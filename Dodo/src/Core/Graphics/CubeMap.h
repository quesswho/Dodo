#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLCubeMap.h"
namespace Dodo { typedef Platform::OpenGLCubeMapTexture CubeMapTexture; }
#elif DD_API_DX11

#endif
