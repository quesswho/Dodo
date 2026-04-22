#include "VulkanTexture.h"
#include "pch.h"
#include <vk_mem_alloc.h>

#include "Core/Math/MathFunc.h"

namespace Dodo::Platform {

    VulkanTexture::VulkanTexture(uchar* data, const TextureProperties& prop, VkDevice device, VmaAllocator allocator,
                                 VkCommandPool commandPool, VkQueue queue)
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
        // FORMAT_RGB16F/32F are expanded to RGBA: the 3-channel float formats have
        // poor GPU coverage, same rationale as the RGB8 case above
        case TextureFormat::FORMAT_RGB16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::FORMAT_RGB32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
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

    void VulkanTexture::TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout,
                                              VkImageLayout newLayout,
                                              uint32_t baseMipLevel, uint32_t levelCount)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.subresourceRange.levelCount = levelCount;
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
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
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
        std::vector<uchar> stagingStorage;

        if (m_TextureProperties.m_Format == TextureFormat::FORMAT_RGB) {
            // RGB is not a guaranteed optimal-tiling format in Vulkan so we pad to RGBA
            stagingStorage.resize(pixelCount * 4);
            for (uint32_t i = 0; i < pixelCount; i++) {
                stagingStorage[i * 4 + 0] = data[i * 3 + 0];
                stagingStorage[i * 4 + 1] = data[i * 3 + 1];
                stagingStorage[i * 4 + 2] = data[i * 3 + 2];
                stagingStorage[i * 4 + 3] = 255;
            }
            uploadData = stagingStorage.data();
            imageSize = (VkDeviceSize)pixelCount * 4;
        } else if (m_TextureProperties.m_Format == TextureFormat::FORMAT_RGB16F) {
            // Expand RGB float (32-bit) to RGBA half-float (16-bit): VK_FORMAT_R16G16B16_SFLOAT
            // has poor GPU coverage so we pad to RGBA, same rationale as the RGB8 case above
            stagingStorage.resize(pixelCount * 4 * sizeof(uint16_t));
            uint16_t* dst = reinterpret_cast<uint16_t*>(stagingStorage.data());
            const float* src = reinterpret_cast<const float*>(data);
            for (uint32_t i = 0; i < pixelCount; i++) {
                dst[i * 4 + 0] = Math::FloatToHalf(src[i * 3 + 0]);
                dst[i * 4 + 1] = Math::FloatToHalf(src[i * 3 + 1]);
                dst[i * 4 + 2] = Math::FloatToHalf(src[i * 3 + 2]);
                dst[i * 4 + 3] = Math::FloatToHalf(1.0f);
            }
            uploadData = stagingStorage.data();
            imageSize = (VkDeviceSize)pixelCount * 4 * sizeof(uint16_t);
        } else if (m_TextureProperties.m_Format == TextureFormat::FORMAT_RGB32F) {
            // Expand RGB float to RGBA float: VK_FORMAT_R32G32B32_SFLOAT has poor GPU coverage
            stagingStorage.resize(pixelCount * 4 * sizeof(float));
            float* dst = reinterpret_cast<float*>(stagingStorage.data());
            const float* src = reinterpret_cast<const float*>(data);
            for (uint32_t i = 0; i < pixelCount; i++) {
                dst[i * 4 + 0] = src[i * 3 + 0];
                dst[i * 4 + 1] = src[i * 3 + 1];
                dst[i * 4 + 2] = src[i * 3 + 2];
                dst[i * 4 + 3] = 1.0f;
            }
            uploadData = stagingStorage.data();
            imageSize = (VkDeviceSize)pixelCount * 4 * sizeof(float);
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

        m_MipLevels = 1 + static_cast<uint32_t>(std::floor(std::log2(static_cast<double>(
            std::max(m_TextureProperties.m_Width, m_TextureProperties.m_Height)))));

        // Create device-local VkImage via VMA
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_TextureProperties.m_Width;
        imageInfo.extent.height = m_TextureProperties.m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo imageAllocCI{};
        imageAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(m_Allocator, &imageInfo, &imageAllocCI, &m_Image, &m_Allocation, nullptr);

        // Transfer: UNDEFINED to TRANSFER_DST (all levels), copy base level, blit remaining mips
        VkCommandBuffer cmd = BeginOneTimeCommands(commandPool);
        TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, m_MipLevels);
        CopyBufferToImage(cmd, stagingBuffer);
        GenerateMipmaps(cmd);
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
        viewInfo.subresourceRange.levelCount = m_MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
    }

    void VulkanTexture::GenerateMipmaps(VkCommandBuffer cmd)
    {
        int32_t mipW = static_cast<int32_t>(m_TextureProperties.m_Width);
        int32_t mipH = static_cast<int32_t>(m_TextureProperties.m_Height);

        for (uint32_t i = 1; i < m_MipLevels; i++) {
            TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i - 1, 1);

            int32_t dstW = std::max(1, mipW / 2);
            int32_t dstH = std::max(1, mipH / 2);

            VkImageBlit blit{};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipW, mipH, 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;
            blit.dstOffsets[0]                 = {0, 0, 0};
            blit.dstOffsets[1]                 = {dstW, dstH, 1};

            vkCmdBlitImage(cmd,
                           m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit, VK_FILTER_LINEAR);

            TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i - 1, 1);

            mipW = dstW;
            mipH = dstH;
        }

        TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels - 1, 1);
    }

    VulkanTexture::~VulkanTexture()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
    }
} // namespace Dodo::Platform
