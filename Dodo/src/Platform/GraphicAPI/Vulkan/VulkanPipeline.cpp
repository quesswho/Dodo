#include "VulkanPipeline.h"
#include "pch.h"

namespace Dodo::Platform {

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
                                   const ShaderAsset& shader, const PipelineDesc& desc)
        : m_Device(device)
    {
        // Setup descriptor set layouts for frame data and texture samplers
        
        // Set 0: FrameData UBO
        VkDescriptorSetLayoutBinding frameUBOBinding{};
        frameUBOBinding.binding = 0;
        frameUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        frameUBOBinding.descriptorCount = 1;
        frameUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo frameSetInfo{};
        frameSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        frameSetInfo.bindingCount = 1;
        frameSetInfo.pBindings = &frameUBOBinding;
        vkCreateDescriptorSetLayout(m_Device, &frameSetInfo, nullptr, &m_FrameSetLayout);

        // Set 1: Texture samplers
        VkDescriptorSetLayoutBinding texBindings[3] = {};
        for (uint32_t i = 0; i < 3; i++) {
            texBindings[i].binding = i;
            texBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            texBindings[i].descriptorCount = 1;
            texBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo texSetInfo{};
        texSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        texSetInfo.bindingCount = 3;
        texSetInfo.pBindings = texBindings;
        vkCreateDescriptorSetLayout(m_Device, &texSetInfo, nullptr, &m_TextureSetLayout);

        // Push constants: DrawData (model matrix + normal matrix)
        // GLSL std430: mat4 = 64B, mat3 = 48B (3 columns aligned to vec4): 112 bytes
        VkPushConstantRange pushConstRange{};
        pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstRange.offset = 0;
        pushConstRange.size = 112;

        VkDescriptorSetLayout setLayouts[] = {m_FrameSetLayout, m_TextureSetLayout};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 2;
        layoutInfo.pSetLayouts = setLayouts;
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

        VkVertexInputAttributeDescription attribDescs[4] = {};
        attribDescs[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};  // position
        attribDescs[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 12};    // texcoord
        attribDescs[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 20}; // normal
        attribDescs[3] = {3, 0, VK_FORMAT_R32G32B32_SFLOAT, 32}; // tangent

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = 4;
        vertexInput.pVertexAttributeDescriptions = attribDescs;

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
        depthStencil.depthWriteEnable = (desc.depthMode != DepthMode::None) ? VK_TRUE : VK_FALSE;
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
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &blendAttachment;

        // Dynamic states, we like to change the viewport and scissor without recreating pipelines
        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        // Dynamic rendering
        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &colorFormat;
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
        vkDestroyDescriptorSetLayout(m_Device, m_TextureSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_Device, m_FrameSetLayout, nullptr);
    }
} // namespace Dodo::Platform
