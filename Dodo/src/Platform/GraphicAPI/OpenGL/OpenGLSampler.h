#pragma once

#include "Core/Graphics/Material/SamplerProperties.h"

namespace Dodo::Platform {

    class OpenGLSampler {
      public:
        OpenGLSampler(const SamplerProperties& settings = SamplerProperties());
        ~OpenGLSampler();

        uint GetSamplerID() const { return m_SamplerID; }

      private:
        uint m_SamplerID;
    };
} // namespace Dodo::Platform