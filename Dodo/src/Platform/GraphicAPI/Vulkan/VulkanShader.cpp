#include "VulkanShader.h"
#include "pch.h"

namespace Dodo::Platform {

    VulkanShader::~VulkanShader() {}

    void VulkanShader::Bind() const {}

    void VulkanShader::Unbind() const {}

    void VulkanShader::SetUniformValue(const char* location, const int value) {}

    void VulkanShader::SetUniformValue(const char* location, const float value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::TVec2<float>& value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::TVec3<float>& value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::TVec4<float>& value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::Mat2& value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::Mat3& value) {}
    void VulkanShader::SetUniformValue(const char* location, const Math::Mat4& value) {}
} // namespace Dodo::Platform