#include "VulkanPipeline.h"
#include "pch.h"

namespace Dodo::Platform {

    VulkanPipeline::~VulkanPipeline() {}

    void VulkanPipeline::Bind() const {}

    void VulkanPipeline::Unbind() const {}

    void VulkanPipeline::SetUniformValue(const char* location, const int value) {}

    void VulkanPipeline::SetUniformValue(const char* location, const float value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::TVec2<float>& value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::TVec3<float>& value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::TVec4<float>& value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::Mat2& value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::Mat3& value) {}
    void VulkanPipeline::SetUniformValue(const char* location, const Math::Mat4& value) {}
} // namespace Dodo::Platform