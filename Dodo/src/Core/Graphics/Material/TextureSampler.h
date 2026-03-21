#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLSampler.h"
namespace Dodo {
    using TextureSampler = Platform::OpenGLSampler;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanSampler.h"
namespace Dodo {
    using TextureSampler = Platform::VulkanSampler;
}
#endif
