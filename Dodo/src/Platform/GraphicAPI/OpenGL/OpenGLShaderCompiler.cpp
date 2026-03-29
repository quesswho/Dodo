#include "OpenGLShaderCompiler.h"
#include "Core/Data/AssetManager.h"
#include <Core/Utilities/Logger.h>
#include <glad/gl.h>
#include <spirv_glsl.hpp>

#include <regex>

namespace Dodo::Platform {

    uint OpenGLShaderCompiler::Compile(ShaderID shaderID, AssetManager& assets)
    {
        ShaderAsset& source = assets.GetShaderAsset(shaderID);
        uint shaderProgram = glCreateProgram();

        for (ShaderStageBinary& stageSource : source.stages) {
            GLenum stageType = GetStageType(stageSource.stage);

            if (stageType == 0) {
                DD_ERR("Shader stage type is not supported in OpenGL!");
                glDeleteProgram(shaderProgram);
                return 0;
            }

            // If ShaderAsset
            if (stageSource.glsl.empty()) {
                stageSource.glsl = TranslateSPIRVToGLSL(stageSource);
            }

            const std::string& glslSource = stageSource.glsl;
            if (glslSource.empty()) {
                DD_ERR("OpenGL shader stage has no GLSL source and SPIR-V translation failed.");
                glDeleteProgram(shaderProgram);
                return 0;
            }

            uint shaderStageId = CompileStage(stageType, glslSource);

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

    std::string OpenGLShaderCompiler::TranslateSPIRVToGLSL(const ShaderStageBinary& source)
    {
        if (source.spirv.empty()) return {};

        try {
            spirv_cross::CompilerGLSL compiler(source.spirv);

            auto options = compiler.get_common_options();
            options.version = 420;
            options.es = false;
            options.vulkan_semantics = false;
            compiler.set_common_options(options);

            // These two settings are particularly important since older GLSL requires a that the sampler is owned by
            // the texture
            compiler.build_dummy_sampler_for_combined_images();
            compiler.build_combined_image_samplers();

            // Flatten separate texture/sampler resources back into explicit OpenGL texture-unit bindings.
            // We use the original texture binding as the combined sampler binding so unit 0 maps to t0/s0, etc.
            for (const auto& combinedSampler : compiler.get_combined_image_samplers()) {
                compiler.set_name(combinedSampler.combined_id, "SPIRV_Cross_Combined" +
                                                                   compiler.get_name(combinedSampler.image_id) +
                                                                   compiler.get_name(combinedSampler.sampler_id));
            }

            return compiler.compile();
        } catch (const std::exception& e) {
            DD_ERR("SPIRV-Cross GLSL translation failed: {}", e.what());
            return {};
        }
    }

    uint OpenGLShaderCompiler::CompileStage(GLenum type, const std::string& source)
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
            DD_ERR("Shader compile error, Code:\n{}\nError: {}", source, errorLog.data());
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    uint OpenGLShaderCompiler::GetStageType(ShaderStage stage)
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
