#pragma once

#include "Core/Data/CubeMapLoader.h"

#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION
// is defined in VulkanRenderAPI.cpp.
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Dodo::Platform {

    class VulkanCubeMap {
      public:
        VulkanCubeMap(const CubeMapData& data, VkDevice device, VmaAllocator allocator,
                      VkCommandPool commandPool, VkQueue queue);
        ~VulkanCubeMap();

        VkImageView GetImageView() const { return m_ImageView; }

      private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VmaAllocator m_Allocator = nullptr;
        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = VK_NULL_HANDLE;
    };
} // namespace Dodo::Platform
