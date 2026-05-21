#pragma once

#include <volk.h>

// Forward-declare VMA types to avoid including vk_mem_alloc.h before VMA_IMPLEMENTATION
// is defined in VulkanRenderAPI.cpp.
typedef struct VmaAllocator_T* VmaAllocator;

namespace Dodo::Platform {

    class VulkanDescriptorAllocator;

    /**
     * Vulkan resources shared across all GpuPass submissions.
     * Passed to Record() so passes can allocate descriptors and create images without
     * a back-pointer to VulkanRenderAPI.
     */
    struct VulkanGpuPassContext {
        VkDevice device;
        VmaAllocator allocator;
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
        VulkanDescriptorAllocator* descriptorAllocator;
        VulkanDescriptorAllocator* bindlessAllocator;
        VkDescriptorSetLayout globalSet0Layout;
        VkDescriptorSetLayout globalSet2Layout;
    };

    /**
     * A self-contained, one-shot GPU workload.
     * Subclasses record commands into the provided command buffer in Record(), hold any
     * temporary GPU resources as members (keeping them alive while the GPU executes), and
     * free those resources and expose results in Finalize() (called on the main thread
     * once the GPU fence signals).
     */
    class VulkanGpuPass {
      public:
        virtual ~VulkanGpuPass() = default;
        virtual void Record(VkCommandBuffer cmd, const VulkanGpuPassContext& ctx) = 0;
        virtual void Finalize() {}
    };

} // namespace Dodo::Platform
