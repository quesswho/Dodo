#pragma once

#include "VulkanDescriptorSet.h"

#include <vector>
#include <volk.h>

namespace Dodo::Platform {

    /**
     * Grows a chain of descriptor pools on demand.
     * Allocations are permanent: no per-frame free, no reset.
     * Owned by VulkanRenderAPI. Replaces all per-pipeline descriptor pools.
     */
    class VulkanDescriptorAllocator {
      public:
        explicit VulkanDescriptorAllocator(VkDevice device, bool updateAfterBind = false);
        ~VulkanDescriptorAllocator();

        VulkanDescriptorAllocator(const VulkanDescriptorAllocator&) = delete;
        VulkanDescriptorAllocator& operator=(const VulkanDescriptorAllocator&) = delete;

        /**
         * Allocates one descriptor set with the given layout and set index.
         * On pool exhaustion, grows a new pool automatically and retries.
         * Returns an invalid VulkanDescriptorSet on total failure.
         */
        VulkanDescriptorSet Allocate(VkDescriptorSetLayout layout, uint32_t setIndex);

      private:
        static constexpr uint32_t k_SetsPerPool         = 512;
        static constexpr uint32_t k_BindlessImageSlots   = 4096;
        static constexpr uint32_t k_BindlessSamplerSlots = 32;

        VkDescriptorPool GrabPool();
        VkDescriptorPool CreatePool();

        bool m_UpdateAfterBind = false;
        VkDevice m_Device;
        VkDescriptorPool m_CurrentPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorPool> m_UsedPools;
        std::vector<VkDescriptorPool> m_FreePools;
    };

} // namespace Dodo::Platform
