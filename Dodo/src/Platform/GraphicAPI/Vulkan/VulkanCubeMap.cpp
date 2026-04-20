#include "VulkanCubeMap.h"
#include "pch.h"
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    VulkanCubeMap::VulkanCubeMap(const CubeMapData& data, VkDevice device, VmaAllocator allocator,
                                 VkCommandPool commandPool, VkQueue queue)
        : m_Device(device), m_Allocator(allocator)
    {
        if (data.faces[0].pixels.empty()) {
            DD_ERR("VulkanCubeMap: CubeMapData is empty (load failed)");
            return;
        }

        const int width = (int)data.faces[0].props.m_Width;
        const int height = (int)data.faces[0].props.m_Height;

        // Each face is RGBA (4 bytes per pixel), guaranteed by CubeMapLoader
        VkDeviceSize faceSize = (VkDeviceSize)width * height * 4;
        VkDeviceSize totalSize = faceSize * 6;

        // Create host-visible staging buffer for all 6 faces
        VkBufferCreateInfo stagingCI{};
        stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingCI.size = totalSize;
        stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocCI{};
        stagingAllocCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAlloc;
        vmaCreateBuffer(m_Allocator, &stagingCI, &stagingAllocCI, &stagingBuffer, &stagingAlloc, nullptr);

        void* mapped;
        vmaMapMemory(m_Allocator, stagingAlloc, &mapped);
        for (int i = 0; i < 6; i++) {
            memcpy(static_cast<uint8_t*>(mapped) + i * faceSize, data.faces[i].pixels.data(), (size_t)faceSize);
        }
        vmaUnmapMemory(m_Allocator, stagingAlloc);

        // Create device-local VkImage (cube-compatible, 6 array layers)
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCI.extent = {(uint32_t)width, (uint32_t)height, 1};
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 6;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo imageAllocCI{};
        imageAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(m_Allocator, &imageCI, &imageAllocCI, &m_Image, &m_Allocation, nullptr);

        // One-time command buffer: transition → copy 6 faces → transition to shader read
        VkCommandBufferAllocateInfo cbAllocInfo{};
        cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAllocInfo.commandPool = commandPool;
        cbAllocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &cbAllocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        // Transition all 6 layers: UNDEFINED → TRANSFER_DST
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &barrier);

        // Copy each face (each is a separate array layer)
        std::array<VkBufferImageCopy, 6> regions;
        for (uint32_t i = 0; i < 6; i++) {
            regions[i] = {};
            regions[i].bufferOffset = i * faceSize;
            regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions[i].imageSubresource.mipLevel = 0;
            regions[i].imageSubresource.baseArrayLayer = i;
            regions[i].imageSubresource.layerCount = 1;
            regions[i].imageExtent = {(uint32_t)width, (uint32_t)height, 1};
        }
        vkCmdCopyBufferToImage(cmd, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions.data());

        // Transition all 6 layers: TRANSFER_DST → SHADER_READ_ONLY
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &barrier);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(m_Device, commandPool, 1, &cmd);

        vmaDestroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);

        // Create cube image view
        VkImageViewCreateInfo viewCI{};
        viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.image = m_Image;
        viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewCI.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.baseMipLevel = 0;
        viewCI.subresourceRange.levelCount = 1;
        viewCI.subresourceRange.baseArrayLayer = 0;
        viewCI.subresourceRange.layerCount = 6;
        vkCreateImageView(m_Device, &viewCI, nullptr, &m_ImageView);
    }

    VulkanCubeMap::~VulkanCubeMap()
    {
        if (m_ImageView) vkDestroyImageView(m_Device, m_ImageView, nullptr);
        if (m_Image) vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
    }

} // namespace Dodo::Platform
