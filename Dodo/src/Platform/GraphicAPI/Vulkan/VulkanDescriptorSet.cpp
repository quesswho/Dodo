#include "VulkanDescriptorSet.h"

namespace Dodo::Platform {

    void VulkanDescriptorSet::Write(uint32_t binding, VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset,
                                    VkDeviceSize range)
    {
        m_BufferInfos.push_back({buffer, offset, range});

        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstBinding = binding;
        w.descriptorCount = 1;
        w.descriptorType = type;
        m_Writes.push_back(w);
    }

    void VulkanDescriptorSet::Write(uint32_t binding, VkImageView view, VkImageLayout layout)
    {
        m_ImageInfos.push_back({VK_NULL_HANDLE, view, layout});

        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstBinding = binding;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        m_Writes.push_back(w);
    }

    void VulkanDescriptorSet::Write(uint32_t binding, VkSampler sampler)
    {
        m_ImageInfos.push_back({sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED});

        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstBinding = binding;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        m_Writes.push_back(w);
    }

    void VulkanDescriptorSet::Write(uint32_t binding, VkImageView view, VkSampler sampler, VkImageLayout layout)
    {
        m_ImageInfos.push_back({sampler, view, layout});

        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstBinding = binding;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        m_Writes.push_back(w);
    }

    void VulkanDescriptorSet::Flush(VkDevice device)
    {
        if (m_Writes.empty()) return;

        // Backfill dstSet and info pointers now that all vectors are stable (no more push_backs).
        // Buffer writes and image writes were pushed to their respective info vectors in the same
        // order as the corresponding writes, so advancing two indices in parallel is correct.
        uint32_t bufferIdx = 0, imageIdx = 0;
        for (auto& w : m_Writes) {
            w.dstSet = m_Handle;
            const bool isBuffer = (w.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                                   w.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                                   w.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (isBuffer)
                w.pBufferInfo = &m_BufferInfos[bufferIdx++];
            else
                w.pImageInfo = &m_ImageInfos[imageIdx++];
        }

        vkUpdateDescriptorSets(device, (uint32_t)m_Writes.size(), m_Writes.data(), 0, nullptr);

        m_Writes.clear();
        m_BufferInfos.clear();
        m_ImageInfos.clear();
    }

    void VulkanDescriptorSet::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                                   VkPipelineBindPoint bindPoint) const
    {
        vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayout, m_SetIndex, 1, &m_Handle, 0, nullptr);
    }

    void VulkanDescriptorSet::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t dynamicOffset,
                                   VkPipelineBindPoint bindPoint) const
    {
        vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayout, m_SetIndex, 1, &m_Handle, 1, &dynamicOffset);
    }

} // namespace Dodo::Platform
