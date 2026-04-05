#pragma once

#include <Core/Common.h>

#include "Core/Graphics/BufferLayout.h"
#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION
// is defined in VulkanRenderAPI.cpp.
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Dodo::Platform {
    class VulkanVertexBuffer {
      public:
        VulkanVertexBuffer(const float* vertices, uint size, const BufferProperties& prop, VkDevice device,
                           VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
        ~VulkanVertexBuffer();

        const BufferProperties& GetBufferProperties() const { return m_BufferProperties; }
        VkBuffer GetBuffer() const { return m_Buffer; }

      private:
        VkCommandBuffer BeginOneTimeCommands(VkCommandPool commandPool) const;
        void EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue) const;

        VkDevice m_Device;
        VmaAllocator m_Allocator;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        const BufferProperties m_BufferProperties;
    };

    class VulkanIndexBuffer {
      public:
        VulkanIndexBuffer(const uint* indices, uint count, VkDevice device, VmaAllocator allocator,
                          VkCommandPool commandPool, VkQueue queue);
        ~VulkanIndexBuffer();

        VkBuffer GetBuffer() const { return m_Buffer; }
        uint GetCount() const { return m_Count; }

      private:
        VkCommandBuffer BeginOneTimeCommands(VkCommandPool commandPool) const;
        void EndOneTimeCommands(VkCommandBuffer cmd, VkCommandPool commandPool, VkQueue queue) const;

        VkDevice m_Device;
        VmaAllocator m_Allocator;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = nullptr;
        const uint m_Count;
    };
} // namespace Dodo::Platform
