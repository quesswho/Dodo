#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLFrameBuffer.h"
namespace Dodo {
    using FrameBuffer = Platform::OpenGLFrameBuffer;
}
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanFrameBuffer.h"
namespace Dodo {
    using FrameBuffer = Platform::VulkanFrameBuffer;
}
#endif
