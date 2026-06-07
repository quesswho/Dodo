#include "VulkanEquirectangularPass.h"

#include "Platform/GraphicAPI/Vulkan/VulkanBuffer.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorAllocator.h"
#include "Platform/GraphicAPI/Vulkan/VulkanModelData.h"
#include "Platform/GraphicAPI/Vulkan/VulkanPipeline.h"
#include "Platform/GraphicAPI/Vulkan/VulkanSampler.h"
#include "Platform/GraphicAPI/Vulkan/VulkanTexture.h"

#include "Core/Graphics/RenderAPITypes.h"

#include <vk_mem_alloc.h>
#include <algorithm>
#include <cmath>

namespace Dodo::Platform {

    VulkanEquirectangularPass::VulkanEquirectangularPass(
        Ref<Texture> equirect, uint faceSize,
        Ref<VulkanPipeline> pipeline, VkDescriptorSetLayout set1Layout,
        Ref<VulkanVertexBuffer> vbo, Ref<VulkanSampler> sampler,
        const VulkanGpuPassContext& ctx)
        : m_Equirect(equirect)
        , m_FaceSize(faceSize)
        , m_MipLevels(1u + static_cast<uint32_t>(std::floor(std::log2(static_cast<double>(faceSize)))))
        , m_Pipeline(pipeline)
        , m_Set1Layout(set1Layout)
        , m_Vbo(vbo)
        , m_Sampler(sampler)
        , m_Device(ctx.device)
        , m_Allocator(ctx.allocator)
        , m_Result(std::make_shared<VulkanCubeMap>(ctx.device, ctx.allocator))
    {
    }

    void VulkanEquirectangularPass::Record(VkCommandBuffer cmd, const VulkanGpuPassContext& ctx)
    {
        // Destination cube image: R16G16B16A16_SFLOAT, 6 layers, full mip chain.
        {
            VkImageCreateInfo ci{};
            ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            ci.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            ci.imageType     = VK_IMAGE_TYPE_2D;
            ci.format        = VK_FORMAT_R16G16B16A16_SFLOAT;
            ci.extent        = {(uint32_t)m_FaceSize, (uint32_t)m_FaceSize, 1};
            ci.mipLevels     = m_MipLevels;
            ci.arrayLayers   = 6;
            ci.samples       = VK_SAMPLE_COUNT_1_BIT;
            ci.tiling        = VK_IMAGE_TILING_OPTIMAL;
            ci.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
            ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(ctx.allocator, &ci, &allocCI, &m_CubeImage, &m_CubeAlloc, nullptr);
        }

        // Per-face 2D views (mip 0 only) used as color attachments during capture.
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

        // Helper to allocate a small persistently-mapped uniform buffer.
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

        // One FrameData UBO per face (camera = proj * captureView[i]).
        // All UBOs are written before command recording so the GPU sees stable data.
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
            gmd.model[0]  = gmd.model[5]  = gmd.model[10]  = gmd.model[15]  = 1.0f;
            gmd.normal[0] = gmd.normal[5] = gmd.normal[10] = gmd.normal[15] = 1.0f;
            memcpy(m_ModelUBO.mapped, &gmd, sizeof(GPUModelData));
        }

        // Allocate descriptor sets.
        // set0: one per face so each draw gets its own view-projection matrix.
        for (int i = 0; i < 6; i++) {
            m_FaceSet0[i] = ctx.bindlessAllocator->Allocate(ctx.globalSet0Layout, 0);
            m_FaceSet0[i].Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                m_FaceUBOs[i].buffer, 0, sizeof(Dodo::FrameData));
            m_FaceSet0[i].Write(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                m_CsmUBO.buffer, 0, sizeof(Dodo::CsmData));
            m_FaceSet0[i].Flush(ctx.device);
        }

        // set1: equirectangular texture and sampler.
        auto* vkTex     = static_cast<VulkanTexture*>(m_Equirect.get());
        auto* vkSampler = static_cast<VulkanSampler*>(m_Sampler.get());
        m_Set1 = ctx.descriptorAllocator->Allocate(m_Set1Layout, 1);
        m_Set1.Write(0, vkTex->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_Set1.Write(1, vkSampler->GetSampler());
        m_Set1.Flush(ctx.device);

        // set2: identity model matrix (dynamic UBO, offset 0).
        m_Set2 = ctx.descriptorAllocator->Allocate(ctx.globalSet2Layout, 2);
        m_Set2.Write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                     m_ModelUBO.buffer, 0, sizeof(GPUModelData));
        m_Set2.Flush(ctx.device);

        // Transition cube mip 0 (all 6 layers): UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier2 b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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
            b.srcStageMask                    = VK_PIPELINE_STAGE_2_NONE;
            b.srcAccessMask                   = 0;
            b.dstStageMask                    = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            b.dstAccessMask                   = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
            dep.imageMemoryBarrierCount = 1;
            dep.pImageMemoryBarriers    = &b;
            vkCmdPipelineBarrier2(cmd, &dep);
        }

        // Pre-transition remaining mip levels to TRANSFER_DST_OPTIMAL ready for blit.
        if (m_MipLevels > 1) {
            VkImageMemoryBarrier2 b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            b.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
            b.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            b.image                           = m_CubeImage;
            b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            b.subresourceRange.baseMipLevel   = 1;
            b.subresourceRange.levelCount     = m_MipLevels - 1;
            b.subresourceRange.baseArrayLayer = 0;
            b.subresourceRange.layerCount     = 6;
            b.srcStageMask                    = VK_PIPELINE_STAGE_2_NONE;
            b.srcAccessMask                   = 0;
            b.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
            b.dstAccessMask                   = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
            dep.imageMemoryBarrierCount = 1;
            dep.pImageMemoryBarriers    = &b;
            vkCmdPipelineBarrier2(cmd, &dep);
        }

        // Transition depth image: UNDEFINED -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier2 b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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
            b.srcStageMask                    = VK_PIPELINE_STAGE_2_NONE;
            b.srcAccessMask                   = 0;
            b.dstStageMask                    = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
            b.dstAccessMask                   = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
            dep.imageMemoryBarrierCount = 1;
            dep.pImageMemoryBarriers    = &b;
            vkCmdPipelineBarrier2(cmd, &dep);
        }

        // Bind state that is constant across all 6 face draws.
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

        // Render each cube face.
        for (int i = 0; i < 6; i++) {
            // Order depth writes from the previous face before the next clear.
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
            depthAtt.sType                        = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAtt.imageView                    = m_DepthView;
            depthAtt.imageLayout                  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAtt.loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAtt.storeOp                      = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAtt.clearValue.depthStencil      = {1.0f, 0};

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

        // Generate mipmaps via a blit chain (all 6 layers blitted together per level).
        if (m_MipLevels == 1) {
            VkImageMemoryBarrier2 b{};
            b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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
            b.srcStageMask                    = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            b.srcAccessMask                   = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            b.dstStageMask                    = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            b.dstAccessMask                   = VK_ACCESS_2_SHADER_READ_BIT;
            VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
            dep.imageMemoryBarrierCount = 1;
            dep.pImageMemoryBarriers    = &b;
            vkCmdPipelineBarrier2(cmd, &dep);
        } else {
            // Transition mip 0 (all 6 layers): COLOR_ATTACHMENT -> TRANSFER_SRC for blitting.
            {
                VkImageMemoryBarrier2 b{};
                b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                b.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                b.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.image                           = m_CubeImage;
                b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                b.subresourceRange.baseMipLevel   = 0;
                b.subresourceRange.levelCount     = 1;
                b.subresourceRange.baseArrayLayer = 0;
                b.subresourceRange.layerCount     = 6;
                b.srcStageMask                    = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                b.srcAccessMask                   = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                b.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
                b.dstAccessMask                   = VK_ACCESS_2_TRANSFER_READ_BIT;
                VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
                dep.imageMemoryBarrierCount = 1;
                dep.pImageMemoryBarriers    = &b;
                vkCmdPipelineBarrier2(cmd, &dep);
            }

            for (uint32_t mip = 1; mip < m_MipLevels; mip++) {
                int32_t srcSize = static_cast<int32_t>(m_FaceSize >> (mip - 1));
                int32_t dstSize = std::max(1, srcSize / 2);

                VkImageBlit blit{};
                blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel       = mip - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount     = 6;
                blit.srcOffsets[1]                 = {srcSize, srcSize, 1};
                blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel       = mip;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount     = 6;
                blit.dstOffsets[1]                 = {dstSize, dstSize, 1};
                vkCmdBlitImage(cmd,
                               m_CubeImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               m_CubeImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &blit, VK_FILTER_LINEAR);

                // Previous mip is now fully read; transition it to shader-readable.
                {
                    VkImageMemoryBarrier2 b{};
                    b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                    b.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    b.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                    b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                    b.image                           = m_CubeImage;
                    b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    b.subresourceRange.baseMipLevel   = mip - 1;
                    b.subresourceRange.levelCount     = 1;
                    b.subresourceRange.baseArrayLayer = 0;
                    b.subresourceRange.layerCount     = 6;
                    b.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
                    b.srcAccessMask                   = VK_ACCESS_2_TRANSFER_READ_BIT;
                    b.dstStageMask                    = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                    b.dstAccessMask                   = VK_ACCESS_2_SHADER_READ_BIT;
                    VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
                    dep.imageMemoryBarrierCount = 1;
                    dep.pImageMemoryBarriers    = &b;
                    vkCmdPipelineBarrier2(cmd, &dep);
                }

                // Promote current mip to TRANSFER_SRC for the next iteration.
                if (mip < m_MipLevels - 1) {
                    VkImageMemoryBarrier2 b{};
                    b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                    b.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    b.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                    b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                    b.image                           = m_CubeImage;
                    b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    b.subresourceRange.baseMipLevel   = mip;
                    b.subresourceRange.levelCount     = 1;
                    b.subresourceRange.baseArrayLayer = 0;
                    b.subresourceRange.layerCount     = 6;
                    b.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
                    b.srcAccessMask                   = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                    b.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
                    b.dstAccessMask                   = VK_ACCESS_2_TRANSFER_READ_BIT;
                    VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
                    dep.imageMemoryBarrierCount = 1;
                    dep.pImageMemoryBarriers    = &b;
                    vkCmdPipelineBarrier2(cmd, &dep);
                }
            }

            // The last mip was the blit destination and remains in TRANSFER_DST; finalize it.
            {
                VkImageMemoryBarrier2 b{};
                b.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                b.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                b.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                b.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                b.image                           = m_CubeImage;
                b.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                b.subresourceRange.baseMipLevel   = m_MipLevels - 1;
                b.subresourceRange.levelCount     = 1;
                b.subresourceRange.baseArrayLayer = 0;
                b.subresourceRange.layerCount     = 6;
                b.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
                b.srcAccessMask                   = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                b.dstStageMask                    = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                b.dstAccessMask                   = VK_ACCESS_2_SHADER_READ_BIT;
                VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
                dep.imageMemoryBarrierCount = 1;
                dep.pImageMemoryBarriers    = &b;
                vkCmdPipelineBarrier2(cmd, &dep);
            }
        }
    }

    void VulkanEquirectangularPass::Finalize()
    {
        // Build the full cube image view covering all 6 layers and all mip levels.
        VkImageView cubeView = VK_NULL_HANDLE;
        {
            VkImageViewCreateInfo vci{};
            vci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vci.image                           = m_CubeImage;
            vci.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
            vci.format                          = VK_FORMAT_R16G16B16A16_SFLOAT;
            vci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            vci.subresourceRange.baseMipLevel   = 0;
            vci.subresourceRange.levelCount     = m_MipLevels;
            vci.subresourceRange.baseArrayLayer = 0;
            vci.subresourceRange.layerCount     = 6;
            vkCreateImageView(m_Device, &vci, nullptr, &cubeView);
        }

        // Hand ownership of the final image to the result cubemap.
        m_Result->Populate(m_CubeImage, m_CubeAlloc, cubeView);
        m_CubeImage = VK_NULL_HANDLE; // ownership transferred, don't destroy in our destructor
        m_CubeAlloc = nullptr;

        // Destroy temporary resources.
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
