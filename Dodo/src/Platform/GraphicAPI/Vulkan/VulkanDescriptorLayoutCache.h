#pragma once

#include <unordered_map>
#include <vector>
#include <volk.h>

namespace Dodo::Platform {

    /**
     * Deduplicates VkDescriptorSetLayout objects across all pipelines.
     * Owned by VulkanRenderAPI. All layouts are destroyed when the cache is destroyed.
     * Not thread-safe: must only be called from the render thread.
     */
    class VulkanDescriptorLayoutCache {
      public:
        explicit VulkanDescriptorLayoutCache(VkDevice device);
        ~VulkanDescriptorLayoutCache();

        VulkanDescriptorLayoutCache(const VulkanDescriptorLayoutCache&) = delete;
        VulkanDescriptorLayoutCache& operator=(const VulkanDescriptorLayoutCache&) = delete;

        /**
         * Returns a cached layout for the given bindings, creating one on first miss.
         * Bindings are sorted by slot index internally so insertion order does not matter.
         */
        VkDescriptorSetLayout GetOrCreate(std::vector<VkDescriptorSetLayoutBinding> bindings);

      private:
        struct LayoutKey {
            std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
            bool operator==(const LayoutKey& other) const;
        };

        struct LayoutKeyHash {
            size_t operator()(const LayoutKey& key) const;
        };

        VkDevice m_Device;
        std::unordered_map<LayoutKey, VkDescriptorSetLayout, LayoutKeyHash> m_Cache;
    };

} // namespace Dodo::Platform
