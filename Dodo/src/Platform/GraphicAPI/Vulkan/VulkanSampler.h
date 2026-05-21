#pragma once

#include "Core/Graphics/Material/SamplerProperties.h"
#include <volk.h>

namespace Dodo::Platform {
    class VulkanSampler {
        friend class VulkanRenderAPI;
      public:
        VulkanSampler(const SamplerProperties& settings, VkDevice device);
        ~VulkanSampler();

        VkSampler GetSampler() const { return m_Sampler; }
        uint32_t GetSamplerIndex() const { return m_SamplerIndex; }

      private:
        VkDevice m_Device;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        uint32_t m_SamplerIndex = 0;
    };
} // namespace Dodo::Platform
