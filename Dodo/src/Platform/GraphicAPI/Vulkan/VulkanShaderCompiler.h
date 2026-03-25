#pragma once

#include <Core/Common.h>
#include <Core/Data/ShaderAsset.h>

namespace Dodo::Platform {
    class VulkanShaderCompiler {
      public:
        static uint Compile(const ShaderAsset& source);

      private:
        static uint CompileStage(const std::string& source);
    };
} // namespace Dodo::Platform
