#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLPostEffect.h"
namespace Dodo { typedef Platform::OpenGLPostEffect PostEffect; }
#elif DD_API_DX11

#endif
