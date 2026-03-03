#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLRenderAPI.h"
namespace Dodo {
    using RenderAPI = Platform::OpenGLRenderAPI;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanRenderAPI.h"
namespace Dodo {
    using RenderAPI = Platform::VulkanRenderAPI;
}

#endif
