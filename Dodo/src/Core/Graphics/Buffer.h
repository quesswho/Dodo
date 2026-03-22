#pragma once

#if defined(DD_API_OPENGL)
#include "Platform/GraphicAPI/OpenGL/OpenGLBuffer.h"
namespace Dodo {
    using VertexBuffer = Platform::OpenGLVertexBuffer;
    using IndexBuffer = Platform::OpenGLIndexBuffer;
} // namespace Dodo
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VulkanBuffer.h"
namespace Dodo {
    using VertexBuffer = Platform::VulkanVertexBuffer;
    using IndexBuffer = Platform::VulkanIndexBuffer;
} // namespace Dodo
#endif
