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

    // Handles to the per-frame UBO buffers owned by VulkanRenderAPI.
    // Set 0 (FrameData + CsmData) is bound globally by VulkanRenderAPI, not per-pipeline.
    struct PipelineUBOHandles {
        static constexpr int maxFrames = 2;
        VkBuffer modelDataBuffers[maxFrames];
        uint32_t modelSlotSize; // aligned slot size for dynamic offset
    };

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const ShaderAsset& shader,
                       const PipelineDesc& desc, const PipelineUBOHandles& ubos,
                       VkDescriptorSetLayout globalSet0Layout,
                       VulkanDescriptorLayoutCache& layoutCache, VulkanDescriptorAllocator& allocator);
        ~VulkanPipeline();

        VkPipeline GetPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetLayout() const { return m_Layout; }
        const PipelineDesc& GetDesc() const { return m_Desc; }

      private:
        // Bind set-2 (ModelData) with the per-draw dynamic offset.
        // No-op if the shader does not declare set-2 bindings.
        void BindObjectSet(VkCommandBuffer cmd, uint32_t frameIdx, uint32_t modelDynamicOffset);

        // Bind set-1 (material textures) using the material's persistent descriptor set.
        // Allocates from m_MaterialPool on first use; re-writes only when matSet.IsDirty().
        // No-op if the shader does not declare set-1 bindings.
        void BindMaterialSet(VkCommandBuffer cmd, VulkanFrameBufferedDescriptorSet& matSet, uint32_t frameIdx,
                             uint32_t frameEpoch, const VkImageView* views, const VkSampler* samplers,
                             const bool* isCubeMap, const bool* isDepth, int maxSlots, VkImageView dummyView,
                             VkSampler dummySampler);

        VkDevice m_Device;
        PipelineDesc m_Desc;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> m_SetLayouts;
        std::vector<DescriptorBindingReflection> m_ShaderBindings;

        bool m_HasSet1 = false; // shader declares set-1 (material textures)
        bool m_HasSet2 = false; // shader declares set-2 (ModelData UBO)

        // Per-pipeline, per-frame descriptor set for set-2 (ModelData).
        // Valid only when m_HasSet2 is true.
        std::array<VulkanDescriptorSet, PipelineUBOHandles::maxFrames> m_Set2{};

        // Non-owning pointers into the shared cache and allocator owned by VulkanRenderAPI.
        VulkanDescriptorLayoutCache* m_LayoutCache = nullptr;
        VulkanDescriptorAllocator* m_Allocator = nullptr;
    };

} // namespace Dodo::Platform
