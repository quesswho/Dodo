#include "VkFrameBuffer.h"
#include "pch.h"

namespace Dodo::Platform {

    VkFrameBuffer::VkFrameBuffer(const FrameBufferProperties& framebufferProp)
        : m_FrameBufferProperties(framebufferProp)
    {
        Create();
    }

    void VkFrameBuffer::Create() {}

    VkFrameBuffer::~VkFrameBuffer() {}

    void VkFrameBuffer::Resize(uint width, uint height) {}
} // namespace Dodo::Platform