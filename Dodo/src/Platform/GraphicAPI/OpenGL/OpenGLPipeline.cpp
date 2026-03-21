#include "OpenGLPipeline.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/System/FileUtils.h"

#include <glad/gl.h>

namespace Dodo::Platform {

    OpenGLPipeline::~OpenGLPipeline()
    {
        glDeleteProgram(m_ShaderID);
    }

    int OpenGLPipeline::GetLocation(const char* location)
    {
        if (m_UniformLocations.find(location) != m_UniformLocations.end()) return m_UniformLocations[location];

        // New location
        const GLint uLoc = glGetUniformLocation(m_ShaderID, location);
        m_UniformLocations.emplace(location, uLoc);
        return uLoc;
    }

    void OpenGLPipeline::SetUniformValue(const char* location, const int value)
    {
        glUniform1i(GetLocation(location), value);
    }

    void OpenGLPipeline::SetUniformValue(const char* location, const float value)
    {
        glUniform1f(GetLocation(location), value);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::TVec2<float>& value)
    {
        glUniform2f(GetLocation(location), value.x, value.y);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::TVec3<float>& value)
    {
        glUniform3f(GetLocation(location), value.x, value.y, value.z);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::TVec4<float>& value)
    {
        glUniform4f(GetLocation(location), value.x, value.y, value.z, value.w);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::Mat2& value)
    {
        glUniformMatrix2fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::Mat3& value)
    {
        glUniformMatrix3fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
    }
    void OpenGLPipeline::SetUniformValue(const char* location, const Math::Mat4& value)
    {
        glUniformMatrix4fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
    }
} // namespace Dodo::Platform