#pragma once

#include <Core/Common.h>
#include <Core/Data/ShaderAsset.h>
#include <volk.h>

#include "Core/Graphics/Pipeline/PipelineDesc.h"

namespace Dodo::Platform {

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const ShaderAsset& shader,
                       const PipelineDesc& desc);
        ~VulkanPipeline();

        VkPipeline GetPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetLayout() const { return m_Layout; }

      private:
        VkDevice m_Device;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_FrameSetLayout = VK_NULL_HANDLE;   // Set 0: FrameData UBO
        VkDescriptorSetLayout m_TextureSetLayout = VK_NULL_HANDLE; // Set 1: albedo/specular/normal samplers
    };

} // namespace Dodo::Platform
