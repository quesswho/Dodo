#pragma once

#include <Core/Common.h>
#include <Core/Graphics/Shader/ShaderSource.h>

namespace Dodo::Platform {
    class VulkanShaderCompiler {
      public:
        static uint Compile(const ShaderSource& source);

      private:
        static uint CompileStage(const std::string& source);
    };
} // namespace Dodo::Platform