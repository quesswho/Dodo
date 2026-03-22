#include "VulkanFrameBuffer.h"
#include "pch.h"

namespace Dodo::Platform {

    VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferProperties& framebufferProp)
        : m_FrameBufferProperties(framebufferProp)
    {
        Create();
    }

    void VulkanFrameBuffer::Create() {}

    VulkanFrameBuffer::~VulkanFrameBuffer() {}

    void VulkanFrameBuffer::Resize(uint width, uint height) {}
} // namespace Dodo::Platform