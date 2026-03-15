#include "VkBuffer.h"
#include "pch.h"

namespace Dodo::Platform {

    // VertexBuffer //

    VkVertexBuffer::VkVertexBuffer(const float* vertices, const uint size, const BufferProperties& prop)
        : m_BufferProperties(prop), m_VBufferID(0)
    {}

    VkVertexBuffer::~VkVertexBuffer() {}

    // IndexBuffer //

    VkIndexBuffer::VkIndexBuffer(const uint* indices, const uint count) : m_Count(count), m_BufferID(0) {}

    VkIndexBuffer::~VkIndexBuffer() {}
} // namespace Dodo::Platform