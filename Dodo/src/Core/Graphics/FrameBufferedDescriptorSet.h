#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLFrameBufferedDescriptorSet.h"
namespace Dodo {
    using FrameBufferedDescriptorSet = Platform::OpenGLFrameBufferedDescriptorSet;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanFrameBufferedDescriptorSet.h"
namespace Dodo {
    using FrameBufferedDescriptorSet = Platform::VulkanFrameBufferedDescriptorSet;
}
#endif
