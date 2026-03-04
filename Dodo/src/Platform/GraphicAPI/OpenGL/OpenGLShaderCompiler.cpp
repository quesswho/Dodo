#include "OpenGLShaderCompiler.h"
#include <Core/Utilities/Logger.h>
#include <glad/gl.h>

#include <regex>

namespace Dodo::Platform {

    uint OpenGLShaderCompiler::Compile(const ShaderSource& source) const
    {
        uint shaderProgram = glCreateProgram();

        for (const ShaderStageSource& stageSource : source.stages) {
            GLenum stageType = GetStageType(stageSource.stage);

            if (stageType == 0) {
                DD_ERR("Shader stage type is not supported in OpenGL!");
                glDeleteProgram(shaderProgram);
                return 0;
            }

            uint shaderStageId = CompileStage(stageType, stageSource.source);

            if (shaderStageId == 0) {
                glDeleteProgram(shaderProgram);
                return 0;
            }

            glAttachShader(shaderProgram, shaderStageId);
            /*
                If a shader object to be deleted is attached to a program object, it will be flagged for deletion, but
                it will not be deleted until it is no longer attached to any program object, for any rendering context
                (i.e., it must be detached from wherever it was attached before it will be deleted). A value of 0 for
                shader will be silently ignored. Source:
                https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
            */
            glDeleteShader(shaderStageId);
        }

        glLinkProgram(shaderProgram);
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> errorLog(maxLength);
            glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, errorLog.data());
            DD_ERR("Program link error: {}", errorLog.data());

            glDeleteProgram(shaderProgram);
            return 0;
        }
        return shaderProgram;
    }

    uint OpenGLShaderCompiler::CompileStage(GLenum type, const std::string& source) const
    {
        uint shader = glCreateShader(type);

        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

            DD_ERR("Shader compile error:\n{}", errorLog.data());
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    uint OpenGLShaderCompiler::GetStageType(ShaderStage stage) const
    {
        switch (stage) {
        case ShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry:
            return GL_GEOMETRY_SHADER;
        case ShaderStage::Compute:
            return GL_COMPUTE_SHADER;
        }
        return 0;
    }
} // namespace Dodo::Platform
