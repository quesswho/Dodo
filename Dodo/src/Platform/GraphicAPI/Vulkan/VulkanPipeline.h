#pragma once

#include <Core/Common.h>
#include <Core/Data/ShaderAsset.h>
#include <volk.h>

#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorAllocator.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorLayoutCache.h"
#include "Platform/GraphicAPI/Vulkan/VulkanFrameBufferedDescriptorSet.h"

#include <array>
#include <vector>

namespace Dodo::Platform {

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const ShaderAsset& shader,
                       const PipelineDesc& desc, VkDescriptorSetLayout globalSet0Layout,
                       VkDescriptorSetLayout globalSet1Layout, VkDescriptorSetLayout globalSet2Layout,
                       VulkanDescriptorLayoutCache& layoutCache, VulkanDescriptorAllocator& allocator);
        ~VulkanPipeline();

        VkPipeline GetPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetLayout() const { return m_Layout; }
        const PipelineDesc& GetDesc() const { return m_Desc; }

      private:
        // Bind set-2 (ModelData) with the per-draw dynamic offset using the shared global set.
        // No-op if the shader does not declare set-2 bindings.
        void BindObjectSet(VkCommandBuffer cmd, const VulkanDescriptorSet& globalSet2, uint32_t modelDynamicOffset);

        VkDevice m_Device;
        PipelineDesc m_Desc;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> m_SetLayouts;
        std::vector<DescriptorBindingReflection> m_ShaderBindings;

        bool m_HasSet2 = false; // shader declares set-2 (ModelData UBO)

        // Non-owning pointers into the shared cache and allocator owned by VulkanRenderAPI.
        VulkanDescriptorLayoutCache* m_LayoutCache = nullptr;
        VulkanDescriptorAllocator* m_Allocator = nullptr;
    };

} // namespace Dodo::Platform
