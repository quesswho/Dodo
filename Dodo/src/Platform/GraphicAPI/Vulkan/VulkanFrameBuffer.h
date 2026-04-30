#pragma once

#include "Core/Graphics/FrameBufferProperties.h"
#include <Core/Common.h>
#include <vector>
#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION is defined
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Dodo::Platform {
    class VulkanFrameBuffer {
      public:
        VulkanFrameBuffer(const FrameBufferProperties& props, VkDevice device, VmaAllocator allocator);
        ~VulkanFrameBuffer();

        // Called by VulkanRenderAPI before rendering into this framebuffer
        void TransitionToRenderTarget(VkCommandBuffer cmd);
        // Called by VulkanRenderAPI after rendering, before sampling as a texture
        void TransitionToReadable(VkCommandBuffer cmd);

        VkImageView GetColorImageView() const { return m_ColorImageView; }
        VkImageView GetDepthImageView() const { return m_DepthImageView; }
        VkImageView GetDepthLayerView(uint32_t layer) const
        {
            return layer < m_DepthLayerViews.size() ? m_DepthLayerViews[layer] : VK_NULL_HANDLE;
        }
        VkSampler GetSampler() const { return m_Sampler; }
        VkExtent2D GetExtent() const { return {m_Properties.m_Width, m_Properties.m_Height}; }
        VkFormat GetColorFormat() const { return VK_FORMAT_R16G16B16A16_SFLOAT; }
        bool HasColor() const
        {
            return m_Properties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL;
        }
        bool IsDepthArray() const
        {
            return m_Properties.m_FrameBufferType == FrameBufferType::FRAMEBUFFER_DEPTH_ARRAY;
        }
        uint32_t GetLayerCount() const { return IsDepthArray() ? m_Properties.m_Layers : 1; }

        inline void Bind() const {}
        void Resize(uint width, uint height);
        inline uint GetTextureHandle() { return 0; }

      private:
        void Create();
        void Destroy();

        VkDevice m_Device;
        VmaAllocator m_Allocator;
        FrameBufferProperties m_Properties;

        VkImage m_ColorImage = VK_NULL_HANDLE;
        VmaAllocation m_ColorAllocation = nullptr;
        VkImageView m_ColorImageView = VK_NULL_HANDLE;
        VkImageLayout m_ColorCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthAllocation = nullptr;
        VkImageView m_DepthImageView = VK_NULL_HANDLE;
        std::vector<VkImageView> m_DepthLayerViews;
        VkImageLayout m_DepthCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkSampler m_Sampler = VK_NULL_HANDLE;
    };
} // namespace Dodo::Platform
