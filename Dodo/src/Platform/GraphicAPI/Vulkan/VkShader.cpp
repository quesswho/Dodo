#include "VkShader.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/System/FileUtils.h"

#include <glad/gl.h>

namespace Dodo::Platform {

    VkShader::~VkShader()
    {
    }

    void VkShader::Bind() const
    {
    }

    void VkShader::Unbind() const
    {
    }

    int VkShader::GetLocation(const char* location)
    {
    }

    void VkShader::SetUniformValue(const char* location, const int value)
    {
    
    }

    void VkShader::SetUniformValue(const char* location, const float value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::TVec2<float>& value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::TVec3<float>& value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::TVec4<float>& value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::Mat2& value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::Mat3& value)
    {
    }
    void VkShader::SetUniformValue(const char* location, const Math::Mat4& value)
    {
    }
} // namespace Dodo::Platform