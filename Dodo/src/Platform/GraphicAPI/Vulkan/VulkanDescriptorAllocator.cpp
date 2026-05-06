#include "VulkanDescriptorAllocator.h"

#include "Core/Utilities/Logger.h"

namespace Dodo::Platform {

    VulkanDescriptorAllocator::VulkanDescriptorAllocator(VkDevice device) : m_Device(device) {}

    VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
    {
        if (m_CurrentPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_Device, m_CurrentPool, nullptr);
        for (auto pool : m_UsedPools)
            vkDestroyDescriptorPool(m_Device, pool, nullptr);
        for (auto pool : m_FreePools)
            vkDestroyDescriptorPool(m_Device, pool, nullptr);
    }

    VulkanDescriptorSet VulkanDescriptorAllocator::Allocate(VkDescriptorSetLayout layout, uint32_t setIndex)
    {
        if (m_CurrentPool == VK_NULL_HANDLE) m_CurrentPool = GrabPool();

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_CurrentPool;
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &layout;

        VkDescriptorSet handle = VK_NULL_HANDLE;
        VkResult result = vkAllocateDescriptorSets(m_Device, &ai, &handle);

        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
            m_UsedPools.push_back(m_CurrentPool);
            m_CurrentPool = GrabPool();
            ai.descriptorPool = m_CurrentPool;
            result = vkAllocateDescriptorSets(m_Device, &ai, &handle);
        }

        if (result != VK_SUCCESS) {
            DD_ERR("VulkanDescriptorAllocator: failed to allocate descriptor set");
            return {};
        }

        VulkanDescriptorSet set;
        set.m_Handle = handle;
        set.m_Layout = layout;
        set.m_SetIndex = setIndex;
        return set;
    }

    VkDescriptorPool VulkanDescriptorAllocator::GrabPool()
    {
        if (!m_FreePools.empty()) {
            VkDescriptorPool pool = m_FreePools.back();
            m_FreePools.pop_back();
            return pool;
        }
        return CreatePool();
    }

    VkDescriptorPool VulkanDescriptorAllocator::CreatePool()
    {
        VkDescriptorPoolSize sizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, k_SetsPerPool * 2},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, k_SetsPerPool},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, k_SetsPerPool * 8},
            {VK_DESCRIPTOR_TYPE_SAMPLER, k_SetsPerPool * 8},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_SetsPerPool * 4},
        };

        VkDescriptorPoolCreateInfo poolCI{};
        poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCI.maxSets = k_SetsPerPool;
        poolCI.poolSizeCount = (uint32_t)std::size(sizes);
        poolCI.pPoolSizes = sizes;

        VkDescriptorPool pool = VK_NULL_HANDLE;
        vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &pool);
        return pool;
    }

} // namespace Dodo::Platform
