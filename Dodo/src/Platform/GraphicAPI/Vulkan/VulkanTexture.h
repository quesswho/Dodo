#pragma once

#include "Core/Graphics/Material/TextureProperties.h"
#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION
// is defined in VulkanRenderAPI.cpp.
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Dodo::Platform {
    class VulkanTexture {
      public:
        VulkanTexture(uchar* data, const TextureProperties& prop, VkDevice device, VmaAllocator allocator,
                      VkCommandPool commandPool, VkQueue queue);
        ~VulkanTexture();

        VkImageView GetImageView() const { return m_ImageView; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }

      private:
        void Init(uchar* data, VkCommandPool commandPool, VkQueue queue);

        VkCommandBuffer BeginOneTimeCommands(VkCommandPool commandPool);
        void EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue);
        void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer);

        TextureProperties m_TextureProperties;
        VkDevice m_Device;
        VmaAllocator m_Allocator;
        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = VK_NULL_HANDLE;
    };
} // namespace Dodo::Platform
