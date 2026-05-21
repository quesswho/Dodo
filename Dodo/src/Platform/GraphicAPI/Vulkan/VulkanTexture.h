#pragma once

#include "Core/Graphics/Material/TextureProperties.h"
#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION
// is defined in VulkanRenderAPI.cpp.
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Dodo::Platform {
    class VulkanTexture {
        friend class VulkanRenderAPI;
      public:
        VulkanTexture(const uchar* data, const TextureProperties& prop, VkDevice device, VmaAllocator allocator,
                      VkCommandBuffer uploadCmdBuf);
        ~VulkanTexture();

        VkImageView GetImageView() const { return m_ImageView; }
        const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }
        uint32_t GetBindlessHandle() const { return m_BindlessHandle; }

        // Destroys the staging buffer once the upload fence has signaled.
        void FinalizeUpload();

      private:
        void Init(const uchar* data, VkCommandBuffer uploadCmdBuf);

        void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   uint32_t baseMipLevel = 0, uint32_t levelCount = 1);
        void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer);
        void GenerateMipmaps(VkCommandBuffer cmd);

        TextureProperties m_TextureProperties;
        VkDevice m_Device;
        VmaAllocator m_Allocator;
        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        uint32_t m_MipLevels = 1;
        uint32_t m_BindlessHandle = 0;
        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VmaAllocation m_StagingAlloc = nullptr;
    };
} // namespace Dodo::Platform
