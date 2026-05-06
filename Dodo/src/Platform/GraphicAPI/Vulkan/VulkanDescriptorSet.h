#pragma once

#include <vector>
#include <volk.h>

namespace Dodo::Platform {

    class VulkanDescriptorAllocator;

    /**
     * A named descriptor set handle that carries its layout and set-index metadata.
     * Supports queuing writes via Write() and submitting them all at once via Flush().
     * Lifetime of the underlying VkDescriptorSet is managed by the allocating pool.
     */
    class VulkanDescriptorSet {
      public:
        bool IsValid() const { return m_Handle != VK_NULL_HANDLE; }
        void Invalidate() { m_Handle = VK_NULL_HANDLE; }

        void Write(uint32_t binding, VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

        void Write(uint32_t binding, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void Write(uint32_t binding, VkSampler sampler);

        void Write(uint32_t binding, VkImageView view, VkSampler sampler,
                   VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void Flush(VkDevice device);

        void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                  VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

        void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t dynamicOffset,
                  VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

      private:
        friend class VulkanDescriptorAllocator;

        VkDescriptorSet m_Handle = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
        uint32_t m_SetIndex = 0;

        std::vector<VkDescriptorBufferInfo> m_BufferInfos;
        std::vector<VkDescriptorImageInfo> m_ImageInfos;
        std::vector<VkWriteDescriptorSet> m_Writes;
    };

} // namespace Dodo::Platform
