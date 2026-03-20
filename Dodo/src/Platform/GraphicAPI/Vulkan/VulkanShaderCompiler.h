#pragma once

#include <Core/Common.h>
#include <Core/Graphics/Pipeline/ShaderSource.h>

namespace Dodo::Platform {
    class VulkanShaderCompiler {
      public:
        static uint Compile(const ShaderSource& source);

      private:
        static uint CompileStage(const std::string& source);
    };
} // namespace Dodo::Platform