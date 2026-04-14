#include "VulkanPipeline.h"
#include "pch.h"

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
                                   const ShaderAsset& shader, const PipelineDesc& desc,
                                   VkDescriptorSetLayout globalSet0Layout)
        : m_Device(device), m_Desc(desc)
    {
        // Build descriptor set layouts from shader reflection data
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

        if (!setBindings.empty()) {
            m_SetLayouts.resize(setBindings.rbegin()->first + 1, VK_NULL_HANDLE);
            for (auto& [set, bindings] : setBindings) {
                // Set-0 is owned by VulkanRenderAPI (global UBOs with dynamic binding).
                // Use the pre-built layout so both sides agree on UNIFORM_BUFFER_DYNAMIC.
                if (set == 0 && globalSet0Layout != VK_NULL_HANDLE) {
                    m_SetLayouts[set] = globalSet0Layout;
                    m_OwnedSet0 = false;
                    continue;
                }
                VkDescriptorSetLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.bindingCount = (uint32_t)bindings.size();
                info.pBindings = bindings.data();
                vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_SetLayouts[set]);
            }

            // Vulkan requires all pSetLayouts entries to be valid handles.
            // Fill any gaps (sets with no bindings) with empty layouts.
            for (auto& layout : m_SetLayouts) {
                if (layout == VK_NULL_HANDLE) {
                    VkDescriptorSetLayoutCreateInfo emptyInfo{};
                    emptyInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    vkCreateDescriptorSetLayout(m_Device, &emptyInfo, nullptr, &layout);
                }
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

        // Mesh layout
        //   vec3 position  (offset  0, 12 bytes)
        //   vec2 texcoord  (offset 12,  8 bytes)
        //   vec3 normal    (offset 20, 12 bytes)
        //   vec3 tangent   (offset 32, 12 bytes)
        //   stride = 44 bytes
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = 11 * sizeof(float);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription allAttribs[4] = {};
        allAttribs[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};  // position
        allAttribs[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 12};    // texcoord
        allAttribs[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 20}; // normal
        allAttribs[3] = {3, 0, VK_FORMAT_R32G32B32_SFLOAT, 32}; // tangent

        std::vector<VkVertexInputAttributeDescription> attribDescs;
        for (const auto& a : allAttribs) {
            if (shader.vertexInputLocations.empty() ||
                std::find(shader.vertexInputLocations.begin(), shader.vertexInputLocations.end(), a.location) !=
                    shader.vertexInputLocations.end()) {
                attribDescs.push_back(a);
            }
        }

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
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
        for (size_t i = 0; i < m_SetLayouts.size(); ++i) {
            // Set-0 may be borrowed from VulkanRenderAPI; only destroy layouts we own.
            if (i == 0 && !m_OwnedSet0) continue;
            vkDestroyDescriptorSetLayout(m_Device, m_SetLayouts[i], nullptr);
        }
    }
} // namespace Dodo::Platform
