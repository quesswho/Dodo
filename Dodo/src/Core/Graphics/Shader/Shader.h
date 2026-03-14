#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLShader.h"
namespace Dodo {
    using Shader = Platform::OpenGLShader;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VkShader.h"
namespace Dodo {
    using Shader = Platform::VkShader;
}
#endif
