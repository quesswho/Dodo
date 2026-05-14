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
        VulkanCubeMap(const CubeMapData& data, VkDevice device, VmaAllocator allocator, VkCommandBuffer uploadCmdBuf);
        // Takes ownership of an image already GPU-resident in SHADER_READ_ONLY_OPTIMAL (no staging needed).
        VulkanCubeMap(VkImage image, VmaAllocation allocation, VkImageView imageView,
                      VkDevice device, VmaAllocator allocator);
        // Creates a pending handle with no image; call Populate() once the GPU pass completes.
        VulkanCubeMap(VkDevice device, VmaAllocator allocator);
        ~VulkanCubeMap();

        VkImageView GetImageView() const { return m_ImageView; }
        bool IsReady() const { return m_Ready; }

        // Called by a GpuPass::Finalize() to supply the finished image and make IsReady() true.
        void Populate(VkImage image, VmaAllocation allocation, VkImageView imageView);

        // Destroys the staging buffer once the upload fence has signaled.
        void FinalizeUpload();

      private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VmaAllocator m_Allocator = nullptr;
        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VmaAllocation m_StagingAlloc = nullptr;
        bool m_Ready = false;
    };
} // namespace Dodo::Platform
