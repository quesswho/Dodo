#include "VulkanDescriptorLayoutCache.h"

#include <algorithm>

namespace Dodo::Platform {

    static size_t HashCombine(size_t seed, size_t value)
    {
        return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    bool VulkanDescriptorLayoutCache::LayoutKey::operator==(const LayoutKey& other) const
    {
        if (m_Bindings.size() != other.m_Bindings.size()) return false;
        if (m_LayoutFlags != other.m_LayoutFlags) return false;
        if (m_BindingFlags != other.m_BindingFlags) return false;
        for (size_t i = 0; i < m_Bindings.size(); i++) {
            const auto& a = m_Bindings[i];
            const auto& b = other.m_Bindings[i];
            if (a.binding != b.binding || a.descriptorType != b.descriptorType ||
                a.descriptorCount != b.descriptorCount || a.stageFlags != b.stageFlags)
                return false;
        }
        return true;
    }

    size_t VulkanDescriptorLayoutCache::LayoutKeyHash::operator()(const LayoutKey& key) const
    {
        size_t seed = key.m_Bindings.size();
        for (const auto& b : key.m_Bindings) {
            seed = HashCombine(seed, b.binding);
            seed = HashCombine(seed, (size_t)b.descriptorType);
            seed = HashCombine(seed, b.descriptorCount);
            seed = HashCombine(seed, b.stageFlags);
        }
        seed = HashCombine(seed, key.m_LayoutFlags);
        for (auto f : key.m_BindingFlags)
            seed = HashCombine(seed, f);
        return seed;
    }

    VulkanDescriptorLayoutCache::VulkanDescriptorLayoutCache(VkDevice device) : m_Device(device) {}

    VulkanDescriptorLayoutCache::~VulkanDescriptorLayoutCache()
    {
        for (auto& [key, layout] : m_Cache)
            vkDestroyDescriptorSetLayout(m_Device, layout, nullptr);
    }

    VkDescriptorSetLayout VulkanDescriptorLayoutCache::GetOrCreate(std::vector<VkDescriptorSetLayoutBinding> bindings)
    {
        return GetOrCreate(std::move(bindings), 0, {});
    }

    VkDescriptorSetLayout VulkanDescriptorLayoutCache::GetOrCreate(
        std::vector<VkDescriptorSetLayoutBinding> bindings,
        VkDescriptorSetLayoutCreateFlags layoutFlags,
        std::vector<VkDescriptorBindingFlags> bindingFlags)
    {
        std::sort(bindings.begin(), bindings.end(),
                  [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
                      return a.binding < b.binding;
                  });

        LayoutKey key{std::move(bindings), layoutFlags, std::move(bindingFlags)};
        auto it = m_Cache.find(key);
        if (it != m_Cache.end()) return it->second;

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{};
        flagsCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        flagsCI.bindingCount = (uint32_t)key.m_BindingFlags.size();
        flagsCI.pBindingFlags = key.m_BindingFlags.empty() ? nullptr : key.m_BindingFlags.data();

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext = key.m_BindingFlags.empty() ? nullptr : &flagsCI;
        info.flags = key.m_LayoutFlags;
        info.bindingCount = (uint32_t)key.m_Bindings.size();
        info.pBindings = key.m_Bindings.data();

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &layout);

        m_Cache.emplace(std::move(key), layout);
        return layout;
    }

} // namespace Dodo::Platform
