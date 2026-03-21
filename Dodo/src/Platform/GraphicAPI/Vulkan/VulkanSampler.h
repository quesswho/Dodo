#pragma once

#include "Core/Graphics/Material/SamplerProperties.h"

namespace Dodo::Platform {
    class VulkanSampler {
      public:
        VulkanSampler(const SamplerProperties& settings = SamplerProperties());
        ~VulkanSampler();

        uint GetSamplerID() const { return m_SamplerID; }

      private:
        uint m_SamplerID;
    };
} // namespace Dodo::Platform