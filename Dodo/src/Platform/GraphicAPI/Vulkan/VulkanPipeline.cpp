#include "VulkanPipeline.h"

#include "Core/Utilities/Logger.h"

#include <map>

namespace Dodo::Platform {

    static VkDescriptorType ToVkDescriptorType(DescriptorType type)
    {
        switch (type) {
        case DescriptorType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::SampledTexture:
        case DescriptorType::SampledCubeMap:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }

    static VkShaderStageFlagBits ToVkStage(ShaderStage stage)
    {
        switch (stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
    }

    static VkCompareOp ToVkDepthOp(DepthMode mode)
    {
        switch (mode) {
        case DepthMode::Never:
            return VK_COMPARE_OP_NEVER;
        case DepthMode::Less:
            return VK_COMPARE_OP_LESS;
        case DepthMode::Equal:
            return VK_COMPARE_OP_EQUAL;
        case DepthMode::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case DepthMode::Greater:
            return VK_COMPARE_OP_GREATER;
        case DepthMode::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case DepthMode::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case DepthMode::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_LESS;
        }
    }

    VulkanPipeline::VulkanPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat,
                                   const ShaderAsset& shader, const PipelineDesc& desc,
                                   VkDescriptorSetLayout globalSet0Layout, VkDescriptorSetLayout globalSet2Layout,
                                   VulkanDescriptorLayoutCache& layoutCache, VulkanDescriptorAllocator& allocator)
        : m_Device(device), m_Desc(desc), m_LayoutCache(&layoutCache), m_Allocator(&allocator)
    {
        // Determine which sets the shader declares via reflection (set 0 is handled globally)
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setBindings;
        for (const auto& b : shader.descriptorBindings) {
            VkDescriptorSetLayoutBinding vkb{};
            vkb.binding = b.binding;
            vkb.descriptorCount = b.count;
            vkb.descriptorType = ToVkDescriptorType(b.type);
            vkb.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            setBindings[b.set].push_back(vkb);
        }

        m_ShaderBindings = shader.descriptorBindings;
        m_HasSet1 = setBindings.count(1) > 0;
        m_HasSet2 = setBindings.count(2) > 0;

        // Set 0 (FrameData + CsmData) is owned and bound globally by VulkanRenderAPI, not per-pipeline.
        // All pipelines use the same global layout so the single bind at frame start is always compatible.
        uint32_t highestSet = 0;
        for (const auto& [s, _] : setBindings)
            highestSet = std::max(highestSet, s);
        m_SetLayouts.resize(std::max(highestSet + 1u, 1u), VK_NULL_HANDLE);
        m_SetLayouts[0] = globalSet0Layout;

        for (auto& [set, bindings] : setBindings) {
            if (set == 0) continue;

            if (set == 2) {
                // Use the shared global Set 2 layout (ModelData UBO, binding 0, UNIFORM_BUFFER_DYNAMIC).
                // The descriptor set itself is owned by VulkanRenderAPI and bound via BindObjectSet.
                m_SetLayouts[2] = globalSet2Layout;
                continue;
            }

            m_SetLayouts[set] = m_LayoutCache->GetOrCreate(bindings);
        }

        // Vulkan requires all pSetLayouts entries to be valid handles.
        // Gaps at slots 1+ get canonical layouts so all pipelines are compatible at those slots.
        for (uint32_t i = 1; i < (uint32_t)m_SetLayouts.size(); i++) {
            if (m_SetLayouts[i] != VK_NULL_HANDLE) continue;

            VkDescriptorSetLayoutBinding canonBinding{};
            canonBinding.binding = 0;
            canonBinding.descriptorCount = 1;
            canonBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            canonBinding.descriptorType =
                (i == 2) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            m_SetLayouts[i] = m_LayoutCache->GetOrCreate({canonBinding});
        }

        // Push constants: DrawData (model matrix + normal matrix)
        // std430: mat4 = 64B, mat4 = 64B: 128 bytes total
        VkPushConstantRange pushConstRange{};
        pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstRange.offset = 0;
        pushConstRange.size = 112;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = (uint32_t)m_SetLayouts.size();
        layoutInfo.pSetLayouts = m_SetLayouts.data();
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstRange;
        vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_Layout);

        // Create Shader modules from SPIR-V binaries
        std::vector<VkShaderModule> modules;
        std::vector<VkPipelineShaderStageCreateInfo> stages;

        for (const auto& stageBinary : shader.stages) {
            if (stageBinary.spirv.empty()) continue;

            VkShaderModuleCreateInfo moduleInfo{};
            moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleInfo.codeSize = stageBinary.spirv.size() * sizeof(uint32_t);
            moduleInfo.pCode = stageBinary.spirv.data();

            VkShaderModule mod;
            if (vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &mod) != VK_SUCCESS) {
                DD_ERR("VulkanPipeline: failed to create shader module for stage {}", (int)stageBinary.stage);
                continue;
            }
            modules.push_back(mod);

            VkPipelineShaderStageCreateInfo stageInfo{};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = ToVkStage(stageBinary.stage);
            stageInfo.module = mod;
            stageInfo.pName = "main"; // TODO: Slang compiles to SPIR-V with "main" as the entry point for some reason
            // stageInfo.pName = stageBinary.entryPoint.c_str();
            stages.push_back(stageInfo);
        }

        // Build vertex input descriptions from shader reflection, sorted by location so offsets
        // accumulate in declaration order.
        auto inputs = shader.vertexInputs;
        std::sort(inputs.begin(), inputs.end(), [](const auto& a, const auto& b) { return a.location < b.location; });

        static constexpr VkFormat kFloatFormats[] = {
            VK_FORMAT_UNDEFINED,        VK_FORMAT_R32_SFLOAT,          VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
        };

        std::vector<VkVertexInputAttributeDescription> attribDescs;
        uint32_t bindingStride = 0;

        if (desc.vertexLayout.m_Stride > 0) {
            bindingStride = (uint32_t)desc.vertexLayout.m_Stride * (uint32_t)sizeof(float);
            for (const auto& vi : inputs) {
                uint32_t c = vi.componentCount;
                VkFormat fmt = (c >= 1 && c <= 4) ? kFloatFormats[c] : VK_FORMAT_UNDEFINED;
                uint32_t attrOffset = 0;
                if (vi.location < (uint32_t)desc.vertexLayout.m_Elements.size())
                    attrOffset = (uint32_t)desc.vertexLayout.m_Elements[vi.location].m_Offset * (uint32_t)sizeof(float);
                attribDescs.push_back({vi.location, 0, fmt, attrOffset});
            }
        } else {
            uint32_t offset = 0;
            for (const auto& vi : inputs) {
                uint32_t c = vi.componentCount;
                VkFormat fmt = (c >= 1 && c <= 4) ? kFloatFormats[c] : VK_FORMAT_UNDEFINED;
                attribDescs.push_back({vi.location, 0, fmt, offset});
                offset += c * (uint32_t)sizeof(float);
            }
            bindingStride = offset;
        }

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = bindingStride;
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = (uint32_t)attribDescs.size();
        vertexInput.pVertexAttributeDescriptions = attribDescs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Rasterizer state
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthClampEnable = desc.depthClip;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.depthBiasEnable = VK_FALSE;

        switch (desc.culling) {
        case CullMode::None:
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::Back:
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::Front:
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        }

        // Multisampling (disabled)
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Depth/stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = (desc.depthMode != DepthMode::None) ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = (desc.depthMode != DepthMode::None && desc.depthWrite) ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = ToVkDepthOp(desc.depthMode);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = desc.stencilTest ? VK_TRUE : VK_FALSE;

        // Color blending
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        switch (desc.blendMode) {
        case BlendMode::AlphaBlend:
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            break;
        case BlendMode::Additive:
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            break;
        default:
            blendAttachment.blendEnable = VK_FALSE;
            break;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = desc.depthOnly ? 0 : 1;
        colorBlending.pAttachments = desc.depthOnly ? nullptr : &blendAttachment;

        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        // Dynamic rendering
        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = desc.depthOnly ? 0 : 1;
        renderingInfo.pColorAttachmentFormats = desc.depthOnly ? nullptr : &colorFormat;
        renderingInfo.depthAttachmentFormat = depthFormat;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = (uint32_t)stages.size();
        pipelineInfo.pStages = stages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_Layout;
        pipelineInfo.renderPass = VK_NULL_HANDLE; // dynamic rendering needs no render pass

        if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
            DD_ERR("VulkanPipeline: failed to create graphics pipeline!");

        for (VkShaderModule mod : modules)
            vkDestroyShaderModule(m_Device, mod, nullptr);
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
        // Descriptor set layouts are owned by VulkanDescriptorLayoutCache.
        // Descriptor set allocations are owned by VulkanDescriptorAllocator.
    }

    void VulkanPipeline::BindObjectSet(VkCommandBuffer cmd, const VulkanDescriptorSet& globalSet2,
                                       uint32_t modelDynamicOffset)
    {
        if (!m_HasSet2) return;
        globalSet2.Bind(cmd, m_Layout, modelDynamicOffset);
    }

    void VulkanPipeline::BindMaterialSet(VkCommandBuffer cmd, VulkanFrameBufferedDescriptorSet& matSet,
                                         uint32_t frameIdx, uint32_t frameEpoch, const VkImageView* views,
                                         const VkSampler* samplers, const bool* isCubeMap, const bool* isDepth,
                                         int maxSlots, VkImageView dummyView, VkSampler dummySampler)
    {
        if (!m_HasSet1) return;
        if (m_SetLayouts.size() <= 1 || m_SetLayouts[1] == VK_NULL_HANDLE) return;

        // Allocate persistent sets from the shared allocator on first use
        if (!matSet.IsAllocated()) {
            for (int i = 0; i < 2; i++) {
                matSet.Get(i) = m_Allocator->Allocate(m_SetLayouts[1], 1);
                if (!matSet.Get(i).IsValid()) return;
            }
        }

        if (matSet.IsDirtyForFrame(frameIdx) && !matSet.WasUpdatedThisEpoch(frameIdx, frameEpoch)) {
            VkSampler firstSampler = VK_NULL_HANDLE;
            for (int s = 0; s < maxSlots && firstSampler == VK_NULL_HANDLE; s++)
                firstSampler = samplers[s];

            VulkanDescriptorSet& s = matSet.Get(frameIdx);
            for (const auto& b : m_ShaderBindings) {
                if (b.set != 1) continue;

                if (b.type == DescriptorType::SampledTexture) {
                    // Only use the pending view if it is a 2D view (never bind a cube view to a Texture2D binding)
                    bool hasPending = b.binding < (uint32_t)maxSlots && views[b.binding] && !isCubeMap[b.binding];
                    VkImageView view = hasPending ? views[b.binding] : dummyView;
                    if (!view) continue;
                    VkImageLayout imgLayout = (hasPending && isDepth[b.binding])
                                                  ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                                  : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    s.Write(b.binding, view, imgLayout);
                } else if (b.type == DescriptorType::SampledCubeMap) {
                    // Only use the pending view if it is a cube view
                    bool hasPending = b.binding < (uint32_t)maxSlots && views[b.binding] && isCubeMap[b.binding];
                    VkImageView view = hasPending ? views[b.binding] : dummyView;
                    if (!view) continue;
                    s.Write(b.binding, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                } else if (b.type == DescriptorType::Sampler) {
                    VkSampler sampler = (b.binding < (uint32_t)maxSlots && samplers[b.binding])
                                            ? samplers[b.binding]
                                            : (firstSampler ? firstSampler : dummySampler);
                    if (!sampler) continue;
                    s.Write(b.binding, sampler);
                } else if (b.type == DescriptorType::CombinedImageSampler) {
                    bool hasPendingView = b.binding < (uint32_t)maxSlots && views[b.binding];
                    VkImageView view = hasPendingView ? views[b.binding] : dummyView;
                    VkSampler sampler = (b.binding < (uint32_t)maxSlots && samplers[b.binding])
                                            ? samplers[b.binding]
                                            : (firstSampler ? firstSampler : dummySampler);
                    if (!view || !sampler) continue;
                    VkImageLayout imgLayout = (hasPendingView && isDepth[b.binding])
                                                  ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                                  : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    s.Write(b.binding, view, sampler, imgLayout);
                }
            }

            s.Flush(m_Device);
            matSet.SetUpdatedEpoch(frameIdx, frameEpoch);
            matSet.ClearDirtyForFrame(frameIdx);
        }

        matSet.Get(frameIdx).Bind(cmd, m_Layout);
    }
} // namespace Dodo::Platform
