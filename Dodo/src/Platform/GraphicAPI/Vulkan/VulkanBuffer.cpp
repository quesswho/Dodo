#include "VulkanBuffer.h"
#include "pch.h"

namespace Dodo::Platform {

    // VertexBuffer //

    VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, const uint size, const BufferProperties& prop)
        : m_BufferProperties(prop), m_VBufferID(0)
    {}

    VulkanVertexBuffer::~VulkanVertexBuffer() {}

    // IndexBuffer //

    VulkanIndexBuffer::VulkanIndexBuffer(const uint* indices, const uint count) : m_Count(count), m_BufferID(0) {}

    VulkanIndexBuffer::~VulkanIndexBuffer() {}
} // namespace Dodo::Platform