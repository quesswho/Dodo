#pragma once

#include "Platform/GraphicAPI/Vulkan/VulkanGpuPass.h"
#include "Platform/GraphicAPI/Vulkan/VulkanBuffer.h"
#include "Platform/GraphicAPI/Vulkan/VulkanCubeMap.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorSet.h"
#include "Platform/GraphicAPI/Vulkan/VulkanPipeline.h"
#include "Platform/GraphicAPI/Vulkan/VulkanSampler.h"
#include "Core/Common.h"

namespace Dodo::Platform {

    /**
     * Convolves an environment cubemap over a hemisphere for each output direction,
     * producing a diffuse irradiance map. The output is a small (typically 32x32)
     * single-mip R16G16B16A16_SFLOAT cubemap. The input env cubemap must already be
     * in SHADER_READ_ONLY_OPTIMAL before Record() is called (guaranteed when CreateIrradianceMap
     * calls WaitAll() before submission). Submitted asynchronously via VulkanGpuPassQueue;
     * the result starts pending (IsReady() == false) and becomes ready after Finalize().
     */
    class VulkanCubemapConvolutionPass : public VulkanGpuPass {
      public:
        VulkanCubemapConvolutionPass(Ref<VulkanCubeMap> envMap, uint faceSize,
                                     Ref<VulkanPipeline> pipeline, VkDescriptorSetLayout set1Layout,
                                     Ref<VulkanVertexBuffer> vbo, Ref<VulkanSampler> sampler,
                                     const VulkanGpuPassContext& ctx);

        void Record(VkCommandBuffer cmd, const VulkanGpuPassContext& ctx) override;
        void Finalize() override;

        Ref<VulkanCubeMap> GetResult() const { return m_Result; }

      private:
        struct MappedBuffer {
            VkBuffer buffer = VK_NULL_HANDLE;
            VmaAllocation allocation = nullptr;
            void* mapped = nullptr;
        };

        Ref<VulkanCubeMap> m_EnvMap;
        uint m_FaceSize;
        Ref<VulkanPipeline> m_Pipeline;
        VkDescriptorSetLayout m_Set1Layout;
        Ref<VulkanVertexBuffer> m_Vbo;
        Ref<VulkanSampler> m_Sampler;

        // GPU resources allocated in Record(), freed in Finalize()
        VkImage       m_CubeImage  = VK_NULL_HANDLE;
        VmaAllocation m_CubeAlloc  = nullptr;
        VkImageView   m_FaceViews[6] = {};
        VkImage       m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthAlloc = nullptr;
        VkImageView   m_DepthView  = VK_NULL_HANDLE;
        MappedBuffer  m_FaceUBOs[6]{};
        MappedBuffer  m_CsmUBO{};
        MappedBuffer  m_ModelUBO{};
        VulkanDescriptorSet m_FaceSet0[6]{};
        VulkanDescriptorSet m_Set1{};
        VulkanDescriptorSet m_Set2{};

        // Cached from context for use in Finalize()
        VkDevice     m_Device    = VK_NULL_HANDLE;
        VmaAllocator m_Allocator = nullptr;

        Ref<VulkanCubeMap> m_Result;
    };

} // namespace Dodo::Platform
