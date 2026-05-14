#pragma once

#include "Platform/GraphicAPI/Vulkan/VulkanGpuPass.h"
#include "Platform/GraphicAPI/Vulkan/VulkanBuffer.h"
#include "Platform/GraphicAPI/Vulkan/VulkanCubeMap.h"
#include "Platform/GraphicAPI/Vulkan/VulkanDescriptorSet.h"
#include "Platform/GraphicAPI/Vulkan/VulkanPipeline.h"
#include "Platform/GraphicAPI/Vulkan/VulkanSampler.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Common.h"

namespace Dodo::Platform {

    /**
     * Converts a lat-long equirectangular HDR texture into a mipmapped cubemap by rendering
     * all 6 faces into a single R16G16B16A16_SFLOAT cube image. Submitted asynchronously via
     * VulkanGpuPassQueue; the result cubemap starts in a pending state (IsReady() == false)
     * and becomes ready once Finalize() is called after the GPU fence signals.
     */
    class VulkanEquirectangularPass : public VulkanGpuPass {
      public:
        VulkanEquirectangularPass(Ref<Texture> equirect, uint faceSize,
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

        Ref<Texture> m_Equirect;
        uint m_FaceSize;
        uint32_t m_MipLevels;
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
