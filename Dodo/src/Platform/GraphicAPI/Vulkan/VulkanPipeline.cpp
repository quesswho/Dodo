#include "VulkanPipeline.h"

#include "Core/Utilities/Logger.h"

#include <map>

namespace Dodo::Platform {

    VkDescriptorType VulkanPipeline::ToVkDescriptorType(DescriptorType type)
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
                                   const ShaderAsset& shader, const PipelineDesc& desc, const PipelineUBOHandles& ubos)
        : m_Device(device), m_Desc(desc)
    {
        // Determine which sets the shader declares via reflection
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
        m_HasSet0 = setBindings.count(0) > 0;
        m_HasSet1 = setBindings.count(1) > 0;
        m_HasSet2 = setBindings.count(2) > 0;

        if (!setBindings.empty()) {
            m_SetLayouts.resize(setBindings.rbegin()->first + 1, VK_NULL_HANDLE);

            for (auto& [set, bindings] : setBindings) {
                if (set == 0) {
                    // Set-0: canonical FrameData layout (binding 0, UNIFORM_BUFFER).
                    // The pipeline owns its own per-frame descriptor sets.
                    VkDescriptorSetLayoutBinding set0Binding{};
                    set0Binding.binding = 0;
                    set0Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    set0Binding.descriptorCount = 1;
                    set0Binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

                    VkDescriptorSetLayoutCreateInfo info{};
                    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    info.bindingCount = 1;
                    info.pBindings = &set0Binding;
                    vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_SetLayouts[0]);

                    VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, PipelineUBOHandles::maxFrames};
                    VkDescriptorPoolCreateInfo poolCI{};
                    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolCI.maxSets = PipelineUBOHandles::maxFrames;
                    poolCI.poolSizeCount = 1;
                    poolCI.pPoolSizes = &poolSize;
                    vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_Set0Pool);

                    VkDescriptorSetLayout layouts[PipelineUBOHandles::maxFrames];
                    for (int i = 0; i < PipelineUBOHandles::maxFrames; i++)
                        layouts[i] = m_SetLayouts[0];

                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = m_Set0Pool;
                    allocInfo.descriptorSetCount = PipelineUBOHandles::maxFrames;
                    allocInfo.pSetLayouts = layouts;
                    vkAllocateDescriptorSets(m_Device, &allocInfo, m_Set0.data());

                    for (int i = 0; i < PipelineUBOHandles::maxFrames; i++) {
                        VkDescriptorBufferInfo frameBI{};
                        frameBI.buffer = ubos.frameDataBuffers[i];
                        frameBI.offset = 0;
                        frameBI.range = VK_WHOLE_SIZE;

                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.dstSet = m_Set0[i];
                        write.dstBinding = 0;
                        write.descriptorCount = 1;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        write.pBufferInfo = &frameBI;
                        vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
                    }
                    continue;
                }

                if (set == 2) {
                    // Set-2: canonical ModelData layout (binding 0, UNIFORM_BUFFER_DYNAMIC).
                    // Dynamic offset selects the per-draw slot in the ring buffer.
                    VkDescriptorSetLayoutBinding set2Binding{};
                    set2Binding.binding = 0;
                    set2Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    set2Binding.descriptorCount = 1;
                    set2Binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

                    VkDescriptorSetLayoutCreateInfo info{};
                    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    info.bindingCount = 1;
                    info.pBindings = &set2Binding;
                    vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_SetLayouts[2]);

                    VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineUBOHandles::maxFrames};
                    VkDescriptorPoolCreateInfo poolCI{};
                    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolCI.maxSets = PipelineUBOHandles::maxFrames;
                    poolCI.poolSizeCount = 1;
                    poolCI.pPoolSizes = &poolSize;
                    vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_Set2Pool);

                    VkDescriptorSetLayout layouts[PipelineUBOHandles::maxFrames];
                    for (int i = 0; i < PipelineUBOHandles::maxFrames; i++)
                        layouts[i] = m_SetLayouts[2];

                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = m_Set2Pool;
                    allocInfo.descriptorSetCount = PipelineUBOHandles::maxFrames;
                    allocInfo.pSetLayouts = layouts;
                    vkAllocateDescriptorSets(m_Device, &allocInfo, m_Set2.data());

                    for (int i = 0; i < PipelineUBOHandles::maxFrames; i++) {
                        VkDescriptorBufferInfo modelBI{};
                        modelBI.buffer = ubos.modelDataBuffers[i];
                        modelBI.offset = 0;
                        modelBI.range = ubos.modelSlotSize;

                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.dstSet = m_Set2[i];
                        write.dstBinding = 0;
                        write.descriptorCount = 1;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                        write.pBufferInfo = &modelBI;
                        vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
                    }
                    continue;
                }

                VkDescriptorSetLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.bindingCount = (uint32_t)bindings.size();
                info.pBindings = bindings.data();
                vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_SetLayouts[set]);
            }

            // Vulkan requires all pSetLayouts entries to be valid handles.
            // Gaps at slots 0 and 2 get canonical layouts so all pipelines are
            // compatible at those slots even when the shader skips them.
            for (uint32_t i = 0; i < (uint32_t)m_SetLayouts.size(); i++) {
                if (m_SetLayouts[i] != VK_NULL_HANDLE) continue;

                VkDescriptorSetLayoutBinding canonBinding{};
                canonBinding.binding = 0;
                canonBinding.descriptorCount = 1;
                canonBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

                VkDescriptorSetLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                if (i == 0) {
                    canonBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    info.bindingCount = 1;
                    info.pBindings = &canonBinding;
                } else if (i == 2) {
                    canonBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    info.bindingCount = 1;
                    info.pBindings = &canonBinding;
                }

                vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_SetLayouts[i]);
            }

            // Create persistent pool for per-material descriptor sets (set-1).
            // Sized by counting actual set-1 bindings so we don't over-allocate.
            if (m_HasSet1) {
                uint32_t sampledImages = 0, samplers = 0, combined = 0;
                for (const auto& b : m_ShaderBindings) {
                    if (b.set != 1) continue;
                    if (b.type == DescriptorType::SampledTexture || b.type == DescriptorType::SampledCubeMap)
                        sampledImages += b.count;
                    else if (b.type == DescriptorType::Sampler)
                        samplers += b.count;
                    else if (b.type == DescriptorType::CombinedImageSampler)
                        combined += b.count;
                }
                uint32_t maxSets = maxMaterials * PipelineUBOHandles::maxFrames;
                std::vector<VkDescriptorPoolSize> matSizes;
                if (sampledImages) matSizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampledImages * maxSets});
                if (samplers)      matSizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLER, samplers * maxSets});
                if (combined)      matSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combined * maxSets});

                VkDescriptorPoolCreateInfo matPoolCI{};
                matPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                matPoolCI.maxSets = maxSets;
                matPoolCI.poolSizeCount = (uint32_t)matSizes.size();
                matPoolCI.pPoolSizes = matSizes.data();
                vkCreateDescriptorPool(m_Device, &matPoolCI, nullptr, &m_MaterialPool);
            }
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
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Every 3 vertices form a triangle
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Rasterizer state
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Polygon rasterization
        rasterizer.lineWidth = 1.0f;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthClampEnable = VK_FALSE;
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
        default: // Opaque, None
            blendAttachment.blendEnable = VK_FALSE;
            break;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = desc.depthOnly ? 0 : 1;
        colorBlending.pAttachments = desc.depthOnly ? nullptr : &blendAttachment;

        // Dynamic states, we like to change the viewport and scissor without recreating pipelines
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

        // Create pipeline
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

        if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            DD_ERR("VulkanPipeline: failed to create graphics pipeline!");
        }

        // Shader modules are only needed during pipeline creation
        for (VkShaderModule mod : modules) {
            vkDestroyShaderModule(m_Device, mod, nullptr);
        }
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
        // Destroying the pool frees all sets allocated from it
        if (m_Set0Pool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_Device, m_Set0Pool, nullptr);
        if (m_Set2Pool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_Device, m_Set2Pool, nullptr);
        if (m_MaterialPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_Device, m_MaterialPool, nullptr);
        for (VkDescriptorSetLayout layout : m_SetLayouts) {
            vkDestroyDescriptorSetLayout(m_Device, layout, nullptr);
        }
    }

    void VulkanPipeline::BindFrameSet(VkCommandBuffer cmd, uint32_t frameIdx)
    {
        if (!m_HasSet0) return;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 0, 1, &m_Set0[frameIdx], 0, nullptr);
    }

    void VulkanPipeline::BindObjectSet(VkCommandBuffer cmd, uint32_t frameIdx, uint32_t modelDynamicOffset)
    {
        if (!m_HasSet2) return;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 2, 1, &m_Set2[frameIdx], 1,
                                &modelDynamicOffset);
    }

    void VulkanPipeline::BindMaterialSet(VkCommandBuffer cmd, VulkanMaterialSet& matSet, uint32_t frameIdx,
                                         const VkImageView* views, const VkSampler* samplers, const bool* isCubeMap,
                                         const bool* isDepth, int maxSlots, VkImageView dummyView,
                                         VkSampler dummySampler)
    {
        if (!m_HasSet1) return;
        if (m_SetLayouts.size() <= 1 || m_SetLayouts[1] == VK_NULL_HANDLE) return;

        // Allocate persistent sets from the material pool on first use
        if (!matSet.IsAllocated()) {
            VkDescriptorSetLayout layouts[PipelineUBOHandles::maxFrames];
            for (int i = 0; i < PipelineUBOHandles::maxFrames; i++)
                layouts[i] = m_SetLayouts[1];

            VkDescriptorSet sets[PipelineUBOHandles::maxFrames] = {};
            VkDescriptorSetAllocateInfo ai{};
            ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool = m_MaterialPool;
            ai.descriptorSetCount = PipelineUBOHandles::maxFrames;
            ai.pSetLayouts = layouts;
            if (vkAllocateDescriptorSets(m_Device, &ai, sets) != VK_SUCCESS) return;
            matSet.Assign(sets[0], sets[1]);
        }

        // Re-write descriptors for all frames only when material textures or samplers changed
        if (matSet.IsDirty()) {
            VkSampler firstSampler = VK_NULL_HANDLE;
            for (int s = 0; s < maxSlots && firstSampler == VK_NULL_HANDLE; s++)
                firstSampler = samplers[s];

            for (int fi = 0; fi < PipelineUBOHandles::maxFrames; fi++) {
                VkDescriptorSet set1 = matSet.Get(fi);

                std::vector<VkWriteDescriptorSet> writes;
                std::vector<VkDescriptorImageInfo> imageInfos;
                imageInfos.reserve(maxSlots);

                for (const auto& b : m_ShaderBindings) {
                    if (b.set != 1) continue;

                    VkWriteDescriptorSet w{};
                    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    w.dstSet = set1;
                    w.dstBinding = b.binding;
                    w.descriptorCount = 1;

                    if (b.type == DescriptorType::SampledTexture) {
                        // Only use the pending view if it is a 2D view (never bind a cube view to a Texture2D binding)
                        bool hasPending = b.binding < (uint32_t)maxSlots && views[b.binding] && !isCubeMap[b.binding];
                        VkImageView view = hasPending ? views[b.binding] : dummyView;
                        if (!view) continue;
                        VkDescriptorImageInfo info{};
                        info.imageView = view;
                        info.imageLayout = (hasPending && isDepth[b.binding])
                                               ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfos.push_back(info);
                        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        w.pImageInfo = &imageInfos.back();
                        writes.push_back(w);
                    } else if (b.type == DescriptorType::SampledCubeMap) {
                        // Only use the pending view if it is a cube view
                        bool hasPending = b.binding < (uint32_t)maxSlots && views[b.binding] && isCubeMap[b.binding];
                        VkImageView view = hasPending ? views[b.binding] : dummyView;
                        if (!view) continue;
                        VkDescriptorImageInfo info{};
                        info.imageView = view;
                        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfos.push_back(info);
                        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        w.pImageInfo = &imageInfos.back();
                        writes.push_back(w);
                    } else if (b.type == DescriptorType::Sampler) {
                        VkSampler sampler = (b.binding < (uint32_t)maxSlots && samplers[b.binding])
                                                ? samplers[b.binding]
                                                : (firstSampler ? firstSampler : dummySampler);
                        if (!sampler) continue;
                        VkDescriptorImageInfo info{};
                        info.sampler = sampler;
                        imageInfos.push_back(info);
                        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                        w.pImageInfo = &imageInfos.back();
                        writes.push_back(w);
                    } else if (b.type == DescriptorType::CombinedImageSampler) {
                        bool hasPendingView = b.binding < (uint32_t)maxSlots && views[b.binding];
                        VkImageView view = hasPendingView ? views[b.binding] : dummyView;
                        VkSampler sampler = (b.binding < (uint32_t)maxSlots && samplers[b.binding])
                                                ? samplers[b.binding]
                                                : (firstSampler ? firstSampler : dummySampler);
                        if (!view || !sampler) continue;
                        VkDescriptorImageInfo info{};
                        info.imageView = view;
                        info.imageLayout = (hasPendingView && isDepth[b.binding])
                                               ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        info.sampler = sampler;
                        imageInfos.push_back(info);
                        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        w.pImageInfo = &imageInfos.back();
                        writes.push_back(w);
                    }
                }

                if (!writes.empty())
                    vkUpdateDescriptorSets(m_Device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
            }
            matSet.ClearDirty();
        }

        VkDescriptorSet set1 = matSet.Get(frameIdx);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 1, 1, &set1, 0, nullptr);
    }

} // namespace Dodo::Platform
