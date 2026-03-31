#pragma once

#include "Core/Graphics/Material/TextureProperties.h"
#include <volk.h>

namespace Dodo::Platform {
    class VulkanTexture {
      public:
        VulkanTexture(uchar* data, const TextureProperties& prop, VkDevice device, VkPhysicalDevice physicalDevice,
                      VkCommandPool commandPool, VkQueue queue);
        ~VulkanTexture();

        VkImageView GetImageView() const { return m_ImageView; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }

      private:
        void Init(uchar* data, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue);

        static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties);
        void CreateBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags memProps, VkBuffer& buffer, VkDeviceMemory& memory);
        VkCommandBuffer BeginOneTimeCommands(VkCommandPool commandPool);
        void EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue);
        void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer);

        TextureProperties m_TextureProperties;
        VkDevice m_Device;
        VkImage m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
    };
} // namespace Dodo::Platform
