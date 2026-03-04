#pragma once

#include <Core/Common.h>
#include <Core/Graphics/Shader/ShaderSource.h>

namespace Dodo::Platform {
    class OpenGLShaderCompiler {
      public:
        OpenGLShaderCompiler() = default;
        ~OpenGLShaderCompiler() = default;

        uint Compile(const ShaderSource& source) const;

      private:
        uint CompileStage(uint type, const std::string& source) const;

        uint GetStageType(ShaderStage stage) const;
    };
} // namespace Dodo::Platform