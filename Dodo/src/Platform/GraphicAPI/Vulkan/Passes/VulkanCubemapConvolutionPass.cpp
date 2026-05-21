#include "VulkanCubemapConvolutionPass.h"

#include "Platform/GraphicAPI/Vulkan/VulkanBuffer.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorAllocator.h"
#include "Platform/GraphicAPI/Vulkan/VulkanModelData.h"
#include "Platform/GraphicAPI/Vulkan/VulkanPipeline.h"
#include "Platform/GraphicAPI/Vulkan/VulkanSampler.h"

#include "Core/Graphics/RenderAPITypes.h"

#include <vk_mem_alloc.h>
#include <cmath>

namespace Dodo::Platform {

    VulkanCubemapConvolutionPass::VulkanCubemapConvolutionPass(
        Ref<VulkanCubeMap> envMap, uint faceSize,
        Ref<VulkanPipeline> pipeline, VkDescriptorSetLayout set1Layout,
        Ref<VulkanVertexBuffer> vbo, Ref<VulkanSampler> sampler,
        const VulkanGpuPassContext& ctx)
        : m_EnvMap(envMap)
        , m_FaceSize(faceSize)
        , m_Pipeline(pipeline)
        , m_Set1Layout(set1Layout)
        , m_Vbo(vbo)
        , m_Sampler(sampler)
        , m_Device(ctx.device)
        , m_Allocator(ctx.allocator)
        , m_Result(std::make_shared<VulkanCubeMap>(ctx.device, ctx.allocator))
    {
    }

    void VulkanCubemapConvolutionPass::Record(VkCommandBuffer cmd, const VulkanGpuPassContext& ctx)
    {
        // Destination irradiance cube image: R16G16B16A16_SFLOAT, 6 layers, single mip.
        {
            VkImageCreateInfo ci{};
            ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            ci.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            ci.imageType     = VK_IMAGE_TYPE_2D;
            ci.format        = VK_FORMAT_R16G16B16A16_SFLOAT;
            ci.extent        = {(uint32_t)m_FaceSize, (uint32_t)m_FaceSize, 1};
            ci.mipLevels     = 1;
            ci.arrayLayers   = 6;
            ci.samples       = VK_SAMPLE_COUNT_1_BIT;
            ci.tiling        = VK_IMAGE_TILING_OPTIMAL;
            ci.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
            ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(ctx.allocator, &ci, &allocCI, &m_CubeImage, &m_CubeAlloc, nullptr);
        }

        // Per-face 2D views used as color attachments during convolution.
        for (uint32_t i = 0; i < 6; i++) {
            VkImageViewCreateInfo vci{};
            vci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vci.image                           = m_CubeImage;
            vci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            vci.format                          = VK_FORMAT_R16G16B16A16_SFLOAT;
            vci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            vci.subresourceRange.baseMipLevel   = 0;
            vci.subresourceRange.levelCount     = 1;
            vci.subresourceRange.baseArrayLayer = i;
            vci.subresourceRange.layerCount     = 1;
            vkCreateImageView(ctx.device, &vci, nullptr, &m_FaceViews[i]);
        }

        // Temporary depth image (shared across all 6 face draws, cleared each time).
        {
            VkImageCreateInfo ci{};
            ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            ci.imageType     = VK_IMAGE_TYPE_2D;
            ci.format        = VK_FORMAT_D32_SFLOAT;
            ci.extent        = {(uint32_t)m_FaceSize, (uint32_t)m_FaceSize, 1};
            ci.mipLevels     = 1;
            ci.arrayLayers   = 1;
            ci.samples       = VK_SAMPLE_COUNT_1_BIT;
            ci.tiling        = VK_IMAGE_TILING_OPTIMAL;
            ci.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
            ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(ctx.allocator, &ci, &allocCI, &m_DepthImage, &m_DepthAlloc, nullptr);

            VkImageViewCreateInfo vci{};
            vci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vci.image                           = m_DepthImage;
            vci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            vci.format                          = VK_FORMAT_D32_SFLOAT;
            vci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
            vci.subresourceRange.baseMipLevel   = 0;
            vci.subresourceRange.levelCount     = 1;
            vci.subresourceRange.baseArrayLayer = 0;
            vci.subresourceRange.layerCount     = 1;
            vkCreateImageView(ctx.device, &vci, nullptr, &m_DepthView);
        }

        auto makeMappedUBO = [&](VkDeviceSize size) -> MappedBuffer {
            MappedBuffer mb{};
            VkBufferCreateInfo bufCI{};
            bufCI.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufCI.size        = size;
            bufCI.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            allocCI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            VmaAllocationInfo info{};
            vmaCreateBuffer(ctx.allocator, &bufCI, &allocCI, &mb.buffer, &mb.allocation, &info);
            mb.mapped = info.pMappedData;
            return mb;
        };

        // One FrameData UBO per face (same capture matrices as the equirectangular pass).
        Math::Mat4       proj = Math::Mat4::Perspective(90.0f, 1.0f, 0.1f, 10.0f);
        const Math::Vec3 origin(0.0f, 0.0f, 0.0f);
        Math::Mat4 captureViews[6] = {
            Math::Mat4::LookAt(origin, Math::Vec3( 1.0f,  0.0f,  0.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3(-1.0f,  0.0f,  0.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  1.0f,  0.0f), Math::Vec3(0.0f,  0.0f,  1.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f, -1.0f,  0.0f), Math::Vec3(0.0f,  0.0f, -1.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  0.0f,  1.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
            Math::Mat4::LookAt(origin, Math::Vec3( 0.0f,  0.0f, -1.0f), Math::Vec3(0.0f, -1.0f,  0.0f)),
        };

        for (int i = 0; i < 6; i++) {
            m_FaceUBOs[i] = makeMappedUBO(sizeof(Dodo::FrameData));
            Dodo::FrameData fd{};
            fd.camera = proj * captureViews[i];
            memcpy(m_FaceUBOs[i].mapped, &fd, sizeof(Dodo::FrameData));
        }

        m_CsmUBO   = makeMappedUBO(sizeof(Dodo::CsmData));
        m_ModelUBO = makeMappedUBO(sizeof(GPUModelData));
        {
            GPUModelData gmd{};
            gmd.model[0]  = gmd.model[5]  = gmd.model[10] = gmd.model[15] = 1.0f;
            gmd.normal[0] = gmd.normal[5] = gmd.normal[10] = gmd.normal[15] = 1.0f;
            memcpy(m_ModelUBO.mapped, &gmd, sizeof(GPUModelData));
        }

        // Descriptor sets: set0 per-face (camera), set1 env cubemap, set2 model matrix.
        for (int i = 0; i < 6; i++) {
            m_FaceSet0[i] = ctx.bindlessAllocator->Allocate(ctx.globalSet0Layout, 0);
            m_FaceSet0[i].Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                m_FaceUBOs[i].buffer, 0, sizeof(Dodo::FrameData));
            m_FaceSet0[i].Write(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                m_CsmUBO.buffer, 0, sizeof(Dodo::CsmData));
            m_FaceSet0[i].Flush(ctx.device);
        }

        m_Set1 = ctx.descriptorAllocator->Allocate(m_Set1Layout, 1);
        m_Set1.Write(0, m_EnvMap->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_Set1.Write(1, static_cast<VulkanSampler*>(m_Sampler.get())->GetSampler());
        m_Set1.Flush(ctx.device);

        m_Set2 = ctx.descriptorAllocator->Allocate(ctx.globalSet2Layout, 2);
        m_Set2.Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                     m_ModelUBO.buffer, 0, sizeof(GPUModelData));
        m_Set2.Flush(ctx.device);

        // Transition irradiance cube (all 6 layers): UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL.
        {
            VkImageMemoryBarrier b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            b.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
            b.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.image                           = m_CubeImage;
            b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            b.subresourceRange.baseMipLevel   = 0;
            b.subresourceRange.levelCount     = 1;
            b.subresourceRange.baseArrayLayer = 0;
            b.subresourceRange.layerCount     = 6;
            b.srcAccessMask                   = 0;
            b.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &b);
        }

        // Transition depth image: UNDEFINED -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
        {
            VkImageMemoryBarrier b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            b.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
            b.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.image                           = m_DepthImage;
            b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
            b.subresourceRange.baseMipLevel   = 0;
            b.subresourceRange.levelCount     = 1;
            b.subresourceRange.baseArrayLayer = 0;
            b.subresourceRange.layerCount     = 1;
            b.srcAccessMask                   = 0;
            b.dstAccessMask                   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &b);
        }

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());
        m_Set1.Bind(cmd, m_Pipeline->GetLayout());
        const uint32_t dynOffset = 0;
        m_Set2.Bind(cmd, m_Pipeline->GetLayout(), dynOffset);

        VkBuffer     vbHandle = m_Vbo->GetBuffer();
        VkDeviceSize vbOffset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vbHandle, &vbOffset);

        VkViewport vp{};
        vp.width    = static_cast<float>(m_FaceSize);
        vp.height   = static_cast<float>(m_FaceSize);
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &vp);

        VkRect2D scissor{{0, 0}, {(uint32_t)m_FaceSize, (uint32_t)m_FaceSize}};
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // Render each cube face with the hemisphere convolution shader.
        for (int i = 0; i < 6; i++) {
            if (i > 0) {
                VkImageMemoryBarrier b{};
                b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                b.oldLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                b.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.image                           = m_DepthImage;
                b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
                b.subresourceRange.baseMipLevel   = 0;
                b.subresourceRange.levelCount     = 1;
                b.subresourceRange.baseArrayLayer = 0;
                b.subresourceRange.layerCount     = 1;
                b.srcAccessMask                   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                b.dstAccessMask                   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &b);
            }

            m_FaceSet0[i].Bind(cmd, m_Pipeline->GetLayout());

            VkRenderingAttachmentInfo colorAtt{};
            colorAtt.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAtt.imageView          = m_FaceViews[i];
            colorAtt.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAtt.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAtt.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
            colorAtt.clearValue.color   = {{0.0f, 0.0f, 0.0f, 1.0f}};

            VkRenderingAttachmentInfo depthAtt{};
            depthAtt.sType                   = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAtt.imageView               = m_DepthView;
            depthAtt.imageLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAtt.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAtt.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAtt.clearValue.depthStencil = {1.0f, 0};

            VkRenderingInfo ri{};
            ri.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
            ri.renderArea           = {{0, 0}, {(uint32_t)m_FaceSize, (uint32_t)m_FaceSize}};
            ri.layerCount           = 1;
            ri.colorAttachmentCount = 1;
            ri.pColorAttachments    = &colorAtt;
            ri.pDepthAttachment     = &depthAtt;

            vkCmdBeginRendering(cmd, &ri);
            vkCmdDraw(cmd, 36, 1, 0, 0);
            vkCmdEndRendering(cmd);
        }

        // Transition irradiance cube to SHADER_READ_ONLY_OPTIMAL for sampling.
        {
            VkImageMemoryBarrier b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            b.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            b.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.image                           = m_CubeImage;
            b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            b.subresourceRange.baseMipLevel   = 0;
            b.subresourceRange.levelCount     = 1;
            b.subresourceRange.baseArrayLayer = 0;
            b.subresourceRange.layerCount     = 6;
            b.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            b.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &b);
        }
    }

    void VulkanCubemapConvolutionPass::Finalize()
    {
        VkImageView cubeView = VK_NULL_HANDLE;
        {
            VkImageViewCreateInfo vci{};
            vci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vci.image                           = m_CubeImage;
            vci.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
            vci.format                          = VK_FORMAT_R16G16B16A16_SFLOAT;
            vci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            vci.subresourceRange.baseMipLevel   = 0;
            vci.subresourceRange.levelCount     = 1;
            vci.subresourceRange.baseArrayLayer = 0;
            vci.subresourceRange.layerCount     = 6;
            vkCreateImageView(m_Device, &vci, nullptr, &cubeView);
        }

        m_Result->Populate(m_CubeImage, m_CubeAlloc, cubeView);
        m_CubeImage = VK_NULL_HANDLE;
        m_CubeAlloc = nullptr;

        for (uint32_t i = 0; i < 6; i++)
            vkDestroyImageView(m_Device, m_FaceViews[i], nullptr);
        vkDestroyImageView(m_Device, m_DepthView, nullptr);
        vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthAlloc);
        m_DepthImage = VK_NULL_HANDLE;
        m_DepthAlloc = nullptr;
        m_DepthView  = VK_NULL_HANDLE;

        for (int i = 0; i < 6; i++)
            vmaDestroyBuffer(m_Allocator, m_FaceUBOs[i].buffer, m_FaceUBOs[i].allocation);
        vmaDestroyBuffer(m_Allocator, m_CsmUBO.buffer, m_CsmUBO.allocation);
        vmaDestroyBuffer(m_Allocator, m_ModelUBO.buffer, m_ModelUBO.allocation);
    }

} // namespace Dodo::Platform
