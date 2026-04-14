#pragma once

#include "Core/Graphics/Material/SamplerProperties.h"
#include <volk.h>

namespace Dodo::Platform {
    class VulkanSampler {
      public:
        VulkanSampler(const SamplerProperties& settings, VkDevice device);
        ~VulkanSampler();

        VkSampler GetSampler() const { return m_Sampler; }

      private:
        VkDevice m_Device;
        VkSampler m_Sampler = VK_NULL_HANDLE;
    };
} // namespace Dodo::Platform
