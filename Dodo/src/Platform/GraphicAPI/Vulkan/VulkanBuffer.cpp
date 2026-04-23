#include "VulkanBuffer.h"

#include <cstring> // memcpy
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    // VertexBuffer //

    VkCommandBuffer VulkanVertexBuffer::BeginOneTimeCommands(VkCommandPool commandPool) const
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);
        return cmd;
    }

    void VulkanVertexBuffer::EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue) const
    {
        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fenceCI{};
        fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        vkCreateFence(m_Device, &fenceCI, nullptr, &fence);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, fence);
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, commandPool, 1, &cmd);
    }

    VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, uint size, const BufferProperties& prop,
                                           VkDevice device, VmaAllocator allocator, VkCommandPool commandPool,
                                           VkQueue queue)
        : m_BufferProperties(prop), m_Device(device), m_Allocator(allocator)
    {
        VkDeviceSize bufSize = (VkDeviceSize)size;

        // Staging buffer (CPU)
        VkBufferCreateInfo stagingCI{};
        stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingCI.size = bufSize;
        stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo stagingAllocCI{};
        stagingAllocCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuf;
        VmaAllocation stagingAlloc;
        vmaCreateBuffer(allocator, &stagingCI, &stagingAllocCI, &stagingBuf, &stagingAlloc, nullptr);

        void* mapped;
        vmaMapMemory(allocator, stagingAlloc, &mapped);
        memcpy(mapped, vertices, (size_t)bufSize);
        vmaUnmapMemory(allocator, stagingAlloc);

        // Device-local buffer
        VkBufferCreateInfo deviceCI{};
        deviceCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        deviceCI.size = bufSize;
        deviceCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        deviceCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo deviceAllocCI{};
        deviceAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateBuffer(allocator, &deviceCI, &deviceAllocCI, &m_Buffer, &m_Allocation, nullptr);

        // Copy staging -> device-local
        VkCommandBuffer cmd = BeginOneTimeCommands(commandPool);
        VkBufferCopy region{0, 0, bufSize};
        vkCmdCopyBuffer(cmd, stagingBuf, m_Buffer, 1, &region);
        EndOneTimeCommands(cmd, commandPool, queue);

        vmaDestroyBuffer(allocator, stagingBuf, stagingAlloc);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }

    // IndexBuffer //

    VkCommandBuffer VulkanIndexBuffer::BeginOneTimeCommands(VkCommandPool commandPool) const
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);
        return cmd;
    }

    void VulkanIndexBuffer::EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue) const
    {
        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fenceCI{};
        fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        vkCreateFence(m_Device, &fenceCI, nullptr, &fence);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, fence);
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, commandPool, 1, &cmd);
    }

    VulkanIndexBuffer::VulkanIndexBuffer(const uint* indices, uint count, VkDevice device, VmaAllocator allocator,
                                         VkCommandPool commandPool, VkQueue queue)
        : m_Count(count), m_Device(device), m_Allocator(allocator)
    {
        VkDeviceSize bufSize = (VkDeviceSize)count * sizeof(uint);

        // Staging buffer (CPU-visible)
        VkBufferCreateInfo stagingCI{};
        stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingCI.size = bufSize;
        stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo stagingAllocCI{};
        stagingAllocCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuf;
        VmaAllocation stagingAlloc;
        vmaCreateBuffer(allocator, &stagingCI, &stagingAllocCI, &stagingBuf, &stagingAlloc, nullptr);

        void* mapped;
        vmaMapMemory(allocator, stagingAlloc, &mapped);
        memcpy(mapped, indices, (size_t)bufSize);
        vmaUnmapMemory(allocator, stagingAlloc);

        // Device-local buffer
        VkBufferCreateInfo deviceCI{};
        deviceCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        deviceCI.size = bufSize;
        deviceCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        deviceCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo deviceAllocCI{};
        deviceAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateBuffer(allocator, &deviceCI, &deviceAllocCI, &m_Buffer, &m_Allocation, nullptr);

        // Copy staging -> device-local
        VkCommandBuffer cmd = BeginOneTimeCommands(commandPool);
        VkBufferCopy region{0, 0, bufSize};
        vkCmdCopyBuffer(cmd, stagingBuf, m_Buffer, 1, &region);
        EndOneTimeCommands(cmd, commandPool, queue);

        vmaDestroyBuffer(allocator, stagingBuf, stagingAlloc);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }

} // namespace Dodo::Platform
