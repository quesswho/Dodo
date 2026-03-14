#pragma once

#ifdef DD_API_OPENGL
#include "Platform/GraphicAPI/OpenGL/OpenGLBuffer.h"
namespace Dodo {
    using VertexBuffer = Platform::OpenGLVertexBuffer;
    using IndexBuffer = Platform::OpenGLIndexBuffer;
} // namespace Dodo
#elif defined(DD_API_VULKAN)
#include "Platform/GraphicAPI/Vulkan/VkBuffer.h"
namespace Dodo {
    using VertexBuffer = Platform::VkVertexBuffer;
    using IndexBuffer = Platform::VkIndexBuffer;
} // namespace Dodo
#endif
