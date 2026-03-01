#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLCubeMap.h"
namespace Dodo {
    using CubeMapTexture = Platform::OpenGLCubeMapTexture;
}
#elif DD_API_DX11

#endif
