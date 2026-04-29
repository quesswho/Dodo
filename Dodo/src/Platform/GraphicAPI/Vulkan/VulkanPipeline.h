#pragma once

#include <Core/Common.h>
#include <Core/Data/ShaderAsset.h>
#include <volk.h>

#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Platform/GraphicAPI/Vulkan/VulkanMaterialSet.h"

#include <array>
#include <vector>

namespace Dodo::Platform {

    // Handles to the per-frame UBO buffers owned by VulkanRenderAPI.
    // Passed to each pipeline at creation so it can write its own set-0 descriptors.
    struct PipelineUBOHandles {
        static constexpr int maxFrames = 2;
        VkBuffer frameDataBuffers[maxFrames];
        VkBuffer modelDataBuffers[maxFrames];
        uint32_t modelSlotSize; // aligned slot size for dynamic offset
    };

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const ShaderAsset& shader,
                       const PipelineDesc& desc, const PipelineUBOHandles& ubos);
        ~VulkanPipeline();

        VkPipeline GetPipeline() const { return m_Pipeline; }
        VkPipelineLayout GetLayout() const { return m_Layout; }
        const PipelineDesc& GetDesc() const { return m_Desc; }

      private:
        static VkDescriptorType ToVkDescriptorType(DescriptorType type);

        // Bind set-0 (FrameData) once per pipeline switch. No dynamic offset.
        // No-op if the shader does not declare set-0 bindings.
        void BindFrameSet(VkCommandBuffer cmd, uint32_t frameIdx);

        // Bind set-2 (ModelData) with the per-draw dynamic offset.
        // No-op if the shader does not declare set-2 bindings.
        void BindObjectSet(VkCommandBuffer cmd, uint32_t frameIdx, uint32_t modelDynamicOffset);

        // Bind set-1 (material textures) using the material's persistent descriptor set.
        // Allocates from m_MaterialPool on first use; re-writes only when matSet.IsDirty().
        // No-op if the shader does not declare set-1 bindings.
        void BindMaterialSet(VkCommandBuffer cmd, VulkanMaterialSet& matSet, uint32_t frameIdx,
                             const VkImageView* views, const VkSampler* samplers, const bool* isCubeMap,
                             const bool* isDepth, int maxSlots, VkImageView dummyView, VkSampler dummySampler);

        VkDevice m_Device;
        PipelineDesc m_Desc;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> m_SetLayouts;
        std::vector<DescriptorBindingReflection> m_ShaderBindings;

        bool m_HasSet0 = false; // shader declares set-0 (FrameData UBO)
        bool m_HasSet1 = false; // shader declares set-1 (material textures)
        bool m_HasSet2 = false; // shader declares set-2 (ModelData UBO)

        // Per-pipeline, per-frame descriptor sets for set-0 (FrameData).
        // Valid only when m_HasSet0 is true.
        VkDescriptorPool m_Set0Pool = VK_NULL_HANDLE;
        std::array<VkDescriptorSet, PipelineUBOHandles::maxFrames> m_Set0{};

        // Per-pipeline, per-frame descriptor sets for set-2 (ModelData).
        // Valid only when m_HasSet2 is true.
        VkDescriptorPool m_Set2Pool = VK_NULL_HANDLE;
        std::array<VkDescriptorSet, PipelineUBOHandles::maxFrames> m_Set2{};

        // Persistent pool for per-material descriptor sets (set-1).
        // All sets are freed together when the pipeline is destroyed.
        static constexpr uint32_t maxMaterials = 256;
        VkDescriptorPool m_MaterialPool = VK_NULL_HANDLE;
    };

} // namespace Dodo::Platform
