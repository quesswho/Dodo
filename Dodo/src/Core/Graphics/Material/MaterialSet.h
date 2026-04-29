#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLMaterialSet.h"
namespace Dodo {
    using MaterialSet = Platform::OpenGLMaterialSet;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanMaterialSet.h"
namespace Dodo {
    using MaterialSet = Platform::VulkanMaterialSet;
}
#endif
