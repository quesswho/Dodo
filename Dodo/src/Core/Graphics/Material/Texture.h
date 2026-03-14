#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLTexture.h"
namespace Dodo {
    using Texture = Platform::OpenGLTexture;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VkTexture.h"
namespace Dodo {
    using Texture = Platform::VkTexture;
}
#endif
