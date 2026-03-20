#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLPipeline.h"
namespace Dodo {
    using Pipeline = Platform::OpenGLPipeline;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanPipeline.h"
namespace Dodo {
    using Pipeline = Platform::VulkanPipeline;
}
#endif
