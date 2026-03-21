#pragma once

#include <Core/Common.h>
#include <volk.h>

#include "Core/Math/Maths.h"

namespace Dodo::Platform {

    class VulkanPipeline {
        friend class VulkanRenderAPI;

      public:
        VulkanPipeline(uint shader) {}
        ~VulkanPipeline();

        void SetUniformValue(const char* location, const int value);
        void SetUniformValue(const char* location, const float value);
        void SetUniformValue(const char* location, const Math::TVec2<float>& value);
        void SetUniformValue(const char* location, const Math::TVec3<float>& value);
        void SetUniformValue(const char* location, const Math::TVec4<float>& value);
        void SetUniformValue(const char* location, const Math::Mat2& value);
        void SetUniformValue(const char* location, const Math::Mat3& value);
        void SetUniformValue(const char* location, const Math::Mat4& value);

      private:
        VkPipeline m_Pipeline;
    };
} // namespace Dodo::Platform