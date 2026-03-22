#pragma once

#include <Core/Common.h>
#include <volk.h>

#include "Core/Math/Maths.h"

namespace Dodo::Platform {

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(const PipelineDesc& desc, uint shaderID) {}
        ~VulkanPipeline();

      private:
        VkPipeline m_Pipeline;
    };
} // namespace Dodo::Platform