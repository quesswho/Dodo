#include "VulkanFrameBuffer.h"
#include "pch.h"
#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    static VkFilter ToVkFilter(SamplerFilter filter)
    {
        switch (filter) {
        case SamplerFilter::MIN_MAG_NEAREST:
        case SamplerFilter::MIN_MAG_MIP_NEAREST:
        case SamplerFilter::MIN_MAG_NEAREST_MIP_LINEAR:
            return VK_FILTER_NEAREST;
        default:
            return VK_FILTER_LINEAR;
        }
    }

    static VkSamplerAddressMode ToVkWrap(SamplerWrapMode mode)
    {
        switch (mode) {
        case SamplerWrapMode::WRAP_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerWrapMode::WRAP_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrapMode::WRAP_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferProperties& props, VkDevice device, VmaAllocator allocator)
        : m_Device(device), m_Allocator(allocator), m_Properties(props)
    {
        Create();
    }

    void VulkanFrameBuffer::Create()
    {
        const uint32_t width = m_Properties.m_Width;
        const uint32_t height = m_Properties.m_Height;
        const bool hasColor = HasColor();

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // Color attachment (COLOR_DEPTH_STENCIL only)
        if (hasColor) {
            VkImageCreateInfo colorCI{};
            colorCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            colorCI.imageType = VK_IMAGE_TYPE_2D;
            colorCI.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            colorCI.extent = {width, height, 1};
            colorCI.mipLevels = 1;
            colorCI.arrayLayers = 1;
            colorCI.samples = VK_SAMPLE_COUNT_1_BIT;
            colorCI.tiling = VK_IMAGE_TILING_OPTIMAL;
            colorCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            colorCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            colorCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vmaCreateImage(m_Allocator, &colorCI, &allocCI, &m_ColorImage, &m_ColorAllocation, nullptr);

            VkImageViewCreateInfo colorViewCI{};
            colorViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorViewCI.image = m_ColorImage;
            colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorViewCI.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            colorViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorViewCI.subresourceRange.baseMipLevel = 0;
            colorViewCI.subresourceRange.levelCount = 1;
            colorViewCI.subresourceRange.baseArrayLayer = 0;
            colorViewCI.subresourceRange.layerCount = 1;
            vkCreateImageView(m_Device, &colorViewCI, nullptr, &m_ColorImageView);

            m_ColorCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        // Depth attachment (always present)
        VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (!hasColor) depthUsage |= VK_IMAGE_USAGE_SAMPLED_BIT; // depth-only fb is sampled directly

        VkImageCreateInfo depthCI{};
        depthCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthCI.imageType = VK_IMAGE_TYPE_2D;
        depthCI.format = VK_FORMAT_D32_SFLOAT;
        depthCI.extent = {width, height, 1};
        depthCI.mipLevels = 1;
        depthCI.arrayLayers = 1;
        depthCI.samples = VK_SAMPLE_COUNT_1_BIT;
        depthCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthCI.usage = depthUsage;
        depthCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vmaCreateImage(m_Allocator, &depthCI, &allocCI, &m_DepthImage, &m_DepthAllocation, nullptr);

        VkImageViewCreateInfo depthViewCI{};
        depthViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthViewCI.image = m_DepthImage;
        depthViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthViewCI.format = VK_FORMAT_D32_SFLOAT;
        depthViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthViewCI.subresourceRange.baseMipLevel = 0;
        depthViewCI.subresourceRange.levelCount = 1;
        depthViewCI.subresourceRange.baseArrayLayer = 0;
        depthViewCI.subresourceRange.layerCount = 1;
        vkCreateImageView(m_Device, &depthViewCI, nullptr, &m_DepthImageView);

        m_DepthCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // Sampler
        const SamplerProperties& sp = m_Properties.m_SamplerProperties;
        VkFilter filter = ToVkFilter(sp.m_Filter);

        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.minFilter = filter;
        samplerCI.magFilter = filter;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCI.addressModeU = ToVkWrap(sp.m_WrapU);
        samplerCI.addressModeV = ToVkWrap(sp.m_WrapV);
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = VK_LOD_CLAMP_NONE;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateSampler(m_Device, &samplerCI, nullptr, &m_Sampler);
    }

    void VulkanFrameBuffer::Destroy()
    {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
        m_Sampler = VK_NULL_HANDLE;

        vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
        m_DepthImageView = VK_NULL_HANDLE;
        vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthAllocation);
        m_DepthImage = VK_NULL_HANDLE;
        m_DepthAllocation = nullptr;

        if (m_ColorImage != VK_NULL_HANDLE) {
            vkDestroyImageView(m_Device, m_ColorImageView, nullptr);
            m_ColorImageView = VK_NULL_HANDLE;
            vmaDestroyImage(m_Allocator, m_ColorImage, m_ColorAllocation);
            m_ColorImage = VK_NULL_HANDLE;
            m_ColorAllocation = nullptr;
        }
    }

    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
        Destroy();
    }

    void VulkanFrameBuffer::TransitionToRenderTarget(VkCommandBuffer cmd)
    {
        if (HasColor()) {
            VkImageMemoryBarrier colorBarrier{};
            colorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            colorBarrier.oldLayout = m_ColorCurrentLayout;
            colorBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            colorBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            colorBarrier.image = m_ColorImage;
            colorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorBarrier.subresourceRange.baseMipLevel = 0;
            colorBarrier.subresourceRange.levelCount = 1;
            colorBarrier.subresourceRange.baseArrayLayer = 0;
            colorBarrier.subresourceRange.layerCount = 1;
            colorBarrier.srcAccessMask = 0;
            colorBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &colorBarrier);
            m_ColorCurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        {
            VkImageMemoryBarrier depthBarrier{};
            depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            depthBarrier.oldLayout = m_DepthCurrentLayout;
            depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.image = m_DepthImage;
            depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthBarrier.subresourceRange.baseMipLevel = 0;
            depthBarrier.subresourceRange.levelCount = 1;
            depthBarrier.subresourceRange.baseArrayLayer = 0;
            depthBarrier.subresourceRange.layerCount = 1;
            depthBarrier.srcAccessMask = 0;
            depthBarrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
                                 0, nullptr, 0, nullptr, 1, &depthBarrier);
            m_DepthCurrentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }

    void VulkanFrameBuffer::TransitionToReadable(VkCommandBuffer cmd)
    {
        if (HasColor()) {
            VkImageMemoryBarrier colorBarrier{};
            colorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            colorBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            colorBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            colorBarrier.image = m_ColorImage;
            colorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorBarrier.subresourceRange.baseMipLevel = 0;
            colorBarrier.subresourceRange.levelCount = 1;
            colorBarrier.subresourceRange.baseArrayLayer = 0;
            colorBarrier.subresourceRange.layerCount = 1;
            colorBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            colorBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier);
            m_ColorCurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        {
            VkImageMemoryBarrier depthBarrier{};
            depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            depthBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.image = m_DepthImage;
            depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthBarrier.subresourceRange.baseMipLevel = 0;
            depthBarrier.subresourceRange.levelCount = 1;
            depthBarrier.subresourceRange.baseArrayLayer = 0;
            depthBarrier.subresourceRange.layerCount = 1;
            depthBarrier.srcAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            depthBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);
            m_DepthCurrentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
    }

    void VulkanFrameBuffer::Resize(uint width, uint height)
    {
        vkDeviceWaitIdle(m_Device);
        Destroy();
        m_Properties.m_Width = width;
        m_Properties.m_Height = height;
        Create();
    }

} // namespace Dodo::Platform
