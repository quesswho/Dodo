#pragma once

#include "Core/Graphics/Material/SamplerProperties.h"

namespace Dodo::Platform {
    class VkSampler {
      public:
        VkSampler(const SamplerProperties& settings = SamplerProperties());
        ~VkSampler();

        uint GetSamplerID() const { return m_SamplerID; }

      private:
        uint m_SamplerID;
    };
} // namespace Dodo::Platform