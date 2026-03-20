#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLShaderCompiler.h"
namespace Dodo {
    using ShaderCompiler = Platform::OpenGLShaderCompiler;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanShaderCompiler.h"
namespace Dodo {
    using ShaderCompiler = Platform::VulkanShaderCompiler;
}
#endif
