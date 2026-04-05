#include "VulkanTexture.h"
#include "pch.h"
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    VulkanTexture::VulkanTexture(uchar* data, const TextureProperties& prop, VkDevice device,
                                 VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
        : m_TextureProperties(prop), m_Device(device), m_Allocator(allocator)
    {
        Init(data, commandPool, queue);
    }

    static VkFormat ToVkFormat(TextureFormat format)
    {
        switch (format) {
        case TextureFormat::FORMAT_RED:
            return VK_FORMAT_R8_UNORM;
        // FORMAT_RGB is expanded to RGBA on upload since VK_FORMAT_R8G8B8_UNORM
        // is an optional format and most GPUs don't support it with optimal tiling
        case TextureFormat::FORMAT_RGB:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::FORMAT_RGBA:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    VkCommandBuffer VulkanTexture::BeginOneTimeCommands(VkCommandPool commandPool)
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

    void VulkanTexture::EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue)
    {
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(m_Device, commandPool, 1, &cmd);
    }

    void VulkanTexture::TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage, dstStage;
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            DD_ERR("VulkanTexture: unsupported layout transition!");
            return;
        }

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanTexture::CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {m_TextureProperties.m_Width, m_TextureProperties.m_Height, 1};

        vkCmdCopyBufferToImage(cmd, buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanTexture::Init(uchar* data, VkCommandPool commandPool, VkQueue queue)
    {
        VkFormat format = ToVkFormat(m_TextureProperties.m_Format);
        uint32_t pixelCount = m_TextureProperties.m_Width * m_TextureProperties.m_Height;

        uchar* uploadData = data;
        VkDeviceSize imageSize;

        if (m_TextureProperties.m_Format == TextureFormat::FORMAT_RGB) {
            // RGB is not a guaranteed optimal-tiling format in Vulkan so we pad to RGBA
            std::vector<uchar> expanded;
            expanded.resize(pixelCount * 4);
            for (uint32_t i = 0; i < pixelCount; i++) {
                expanded[i * 4 + 0] = data[i * 3 + 0];
                expanded[i * 4 + 1] = data[i * 3 + 1];
                expanded[i * 4 + 2] = data[i * 3 + 2];
                expanded[i * 4 + 3] = 255; // Set alpha to opaque
            }
            uploadData = expanded.data();
            imageSize = (VkDeviceSize)pixelCount * 4;
        } else if (m_TextureProperties.m_Format == TextureFormat::FORMAT_RED) {
            imageSize = (VkDeviceSize)pixelCount;
        } else {
            imageSize = (VkDeviceSize)pixelCount * 4;
        }

        // Upload pixel data via a host-visible staging buffer
        VkBufferCreateInfo stagingCI{};
        stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingCI.size = imageSize;
        stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo stagingAllocCI{};
        stagingAllocCI.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAlloc;
        vmaCreateBuffer(m_Allocator, &stagingCI, &stagingAllocCI, &stagingBuffer, &stagingAlloc, nullptr);

        void* mapped;
        vmaMapMemory(m_Allocator, stagingAlloc, &mapped);
        memcpy(mapped, uploadData, (size_t)imageSize);
        vmaUnmapMemory(m_Allocator, stagingAlloc);

        // Create device-local VkImage via VMA
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_TextureProperties.m_Width;
        imageInfo.extent.height = m_TextureProperties.m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo imageAllocCI{};
        imageAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(m_Allocator, &imageInfo, &imageAllocCI, &m_Image, &m_Allocation, nullptr);

        // Transfer: UNDEFINED to TRANSFER_DST, copy, TRANSFER_DST to SHADER_READ_ONLY
        VkCommandBuffer cmd = BeginOneTimeCommands(commandPool);
        TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(cmd, stagingBuffer);
        TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        EndOneTimeCommands(cmd, commandPool, queue);

        vmaDestroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);

        // Create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
    }

    VulkanTexture::~VulkanTexture()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
    }
} // namespace Dodo::Platform
