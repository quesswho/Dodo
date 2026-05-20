#include "VulkanTexture.h"

#include "Core/Math/MathFunc.h"
#include "Core/Utilities/Logger.h"

#include <vector>
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    VulkanTexture::VulkanTexture(const uchar* data, const TextureProperties& prop, VkDevice device,
                                 VmaAllocator allocator, VkCommandBuffer uploadCmdBuf)
        : m_TextureProperties(prop), m_Device(device), m_Allocator(allocator)
    {
        Init(data, uploadCmdBuf);
    }

    static VkFormat ToVkFormat(TextureFormat format)
    {
        switch (format) {
        case TextureFormat::FORMAT_RED:
            return VK_FORMAT_R8_UNORM;
        // FORMAT_RGB is not produced by TextureLoader (loader always pads to RGBA via STBI_rgb_alpha).
        // Kept as a fallback: if somehow FORMAT_RGB data arrives, map to R8G8B8A8 since
        // VK_FORMAT_R8G8B8_UNORM optimal tiling is not guaranteed by the Vulkan spec.
        case TextureFormat::FORMAT_RGB:
        case TextureFormat::FORMAT_RGBA:
            return VK_FORMAT_R8G8B8A8_UNORM;
        // FORMAT_RGB16F/32F are expanded to RGBA: the 3-channel float formats have
        // poor GPU coverage, same rationale as the RGB8 case above
        case TextureFormat::FORMAT_RGB16F:
        case TextureFormat::FORMAT_RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::FORMAT_RGB32F:
        case TextureFormat::FORMAT_RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::FORMAT_BC1_RGB_UNORM:  return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case TextureFormat::FORMAT_BC3_RGBA_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::FORMAT_BC5_RG_UNORM:   return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::FORMAT_BC7_RGBA_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
        default:
            DD_ERR("VulkanTexture: unsupported texture format!");
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    static uint32_t BytesPerPixel(TextureFormat fmt)
    {
        switch (fmt) {
        case TextureFormat::FORMAT_RED:     return 1;
        case TextureFormat::FORMAT_RGBA:    return 4;
        case TextureFormat::FORMAT_RGBA16F: return 8;
        default:                            return 4;
        }
    }

    static bool IsCompressedFormat(TextureFormat fmt)
    {
        switch (fmt) {
        case TextureFormat::FORMAT_BC1_RGB_UNORM:
        case TextureFormat::FORMAT_BC3_RGBA_UNORM:
        case TextureFormat::FORMAT_BC5_RG_UNORM:
        case TextureFormat::FORMAT_BC7_RGBA_UNORM:
            return true;
        default:
            return false;
        }
    }

    static VkDeviceSize CompressedMipBytes(TextureFormat fmt, uint32_t w, uint32_t h)
    {
        uint32_t blockBytes = (fmt == TextureFormat::FORMAT_BC1_RGB_UNORM) ? 8u : 16u;
        return (VkDeviceSize)((w + 3u) / 4u) * ((h + 3u) / 4u) * blockBytes;
    }

    void VulkanTexture::TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
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
        if (m_TextureProperties.m_MipmapMode != MipmapMode::Preloaded) {
            VkBufferImageCopy region{};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = {m_TextureProperties.m_Width, m_TextureProperties.m_Height, 1};
            vkCmdCopyBufferToImage(cmd, buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            return;
        }

        std::vector<VkBufferImageCopy> regions(m_MipLevels);
        VkDeviceSize offset = 0;
        for (uint32_t i = 0; i < m_MipLevels; i++) {
            uint32_t w = (std::max)(1u, m_TextureProperties.m_Width  >> i);
            uint32_t h = (std::max)(1u, m_TextureProperties.m_Height >> i);
            regions[i].bufferOffset = offset;
            regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions[i].imageSubresource.mipLevel = i;
            regions[i].imageSubresource.layerCount = 1;
            regions[i].imageExtent = {w, h, 1};
            if (IsCompressedFormat(m_TextureProperties.m_Format))
                offset += CompressedMipBytes(m_TextureProperties.m_Format, w, h);
            else
                offset += (VkDeviceSize)w * h * BytesPerPixel(m_TextureProperties.m_Format);
        }
        vkCmdCopyBufferToImage(cmd, buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               m_MipLevels, regions.data());
    }

    void VulkanTexture::Init(const uchar* data, VkCommandBuffer uploadCmdBuf)
    {
        VkFormat format = ToVkFormat(m_TextureProperties.m_Format);
        uint32_t pixelCount = m_TextureProperties.m_Width * m_TextureProperties.m_Height;

        const uchar* uploadData = data;
        VkDeviceSize imageSize;
        std::vector<uchar> stagingStorage;

        if (IsCompressedFormat(m_TextureProperties.m_Format)) {
            imageSize = CompressedMipBytes(m_TextureProperties.m_Format,
                                          m_TextureProperties.m_Width,
                                          m_TextureProperties.m_Height);
        } else {
            switch (m_TextureProperties.m_Format) {
            case TextureFormat::FORMAT_RGBA16F:
                imageSize = (VkDeviceSize)pixelCount * 4 * sizeof(uint16_t);
                break;
            case TextureFormat::FORMAT_RGBA:
                imageSize = (VkDeviceSize)pixelCount * 4;
                break;
            case TextureFormat::FORMAT_RED:
                imageSize = (VkDeviceSize)pixelCount;
                break;
            default:
                DD_ERR("VulkanTexture: unsupported texture format!");
                return;
            }
        }

        switch (m_TextureProperties.m_MipmapMode) {
        case MipmapMode::None:
            m_MipLevels = 1;
            break;
        case MipmapMode::Generated:
            m_MipLevels = 1 + static_cast<uint32_t>(std::floor(std::log2(static_cast<double>(
                                  (std::max)(m_TextureProperties.m_Width, m_TextureProperties.m_Height)))));
            break;
        case MipmapMode::Preloaded:
            m_MipLevels = (std::max)(1u, m_TextureProperties.m_MipLevels);
            break;
        }

        VkDeviceSize stagingSize = imageSize;
        if (m_TextureProperties.m_MipmapMode == MipmapMode::Preloaded) {
            stagingSize = 0;
            for (uint32_t i = 0; i < m_MipLevels; i++) {
                uint32_t w = (std::max)(1u, m_TextureProperties.m_Width  >> i);
                uint32_t h = (std::max)(1u, m_TextureProperties.m_Height >> i);
                if (IsCompressedFormat(m_TextureProperties.m_Format))
                    stagingSize += CompressedMipBytes(m_TextureProperties.m_Format, w, h);
                else
                    stagingSize += (VkDeviceSize)w * h * BytesPerPixel(m_TextureProperties.m_Format);
            }
        }

        // Upload pixel data via a host-visible staging buffer (kept alive until FinalizeUpload)
        VkBufferCreateInfo stagingCI{};
        stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingCI.size = stagingSize;
        stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo stagingAllocCI{};
        stagingAllocCI.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo stagingAllocInfo;
        vmaCreateBuffer(m_Allocator, &stagingCI, &stagingAllocCI, &m_StagingBuffer, &m_StagingAlloc,
                        &stagingAllocInfo);
        memcpy(stagingAllocInfo.pMappedData, uploadData, (size_t)stagingSize);

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
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (m_TextureProperties.m_MipmapMode == MipmapMode::Generated)
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo imageAllocCI{};
        imageAllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(m_Allocator, &imageInfo, &imageAllocCI, &m_Image, &m_Allocation, nullptr);

        // Record transfer commands into the shared upload command buffer.
        // The pipeline barriers ensure ordering within the graphics queue so the shaders
        // see the completed upload even without an explicit semaphore between submissions.
        TransitionImageLayout(uploadCmdBuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
                              m_MipLevels);
        CopyBufferToImage(uploadCmdBuf, m_StagingBuffer);
        if (m_TextureProperties.m_MipmapMode == MipmapMode::Generated) {
            GenerateMipmaps(uploadCmdBuf);
        } else {
            TransitionImageLayout(uploadCmdBuf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, m_MipLevels);
        }

        // Create the image view now: VkImageView creation is purely metadata and does not
        // require the GPU upload to have completed yet.
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

    void VulkanTexture::FinalizeUpload()
    {
        if (m_StagingBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(m_Allocator, m_StagingBuffer, m_StagingAlloc);
            m_StagingBuffer = VK_NULL_HANDLE;
            m_StagingAlloc = nullptr;
        }
    }

    void VulkanTexture::GenerateMipmaps(VkCommandBuffer cmd)
    {
        int32_t mipW = static_cast<int32_t>(m_TextureProperties.m_Width);
        int32_t mipH = static_cast<int32_t>(m_TextureProperties.m_Height);

        for (uint32_t i = 1; i < m_MipLevels; i++) {
            TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  i - 1, 1);

            int32_t dstW = (std::max)(1, mipW / 2);
            int32_t dstH = (std::max)(1, mipH / 2);

            VkImageBlit blit{};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipW, mipH, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {dstW, dstH, 1};

            vkCmdBlitImage(cmd, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

            TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  i - 1, 1);

            mipW = dstW;
            mipH = dstH;
        }

        TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              m_MipLevels - 1, 1);
    }

    VulkanTexture::~VulkanTexture()
    {
        FinalizeUpload(); // no-op if already finalized; safety net for early destruction
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
    }
} // namespace Dodo::Platform
