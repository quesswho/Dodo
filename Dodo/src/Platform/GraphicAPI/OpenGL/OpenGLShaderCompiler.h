#pragma once

#include <Core/Common.h>
#include <Core/Data/AssetTypes.h>
#include <Core/Data/ShaderAsset.h>

namespace Dodo {
    class AssetManager;
}

namespace Dodo::Platform {
    class OpenGLShaderCompiler {
      public:
        static uint Compile(ShaderID shaderID, AssetManager& assets);

      private:
        static uint CompileStage(uint type, const std::string& source);
        static std::string TranslateSPIRVToGLSL(const ShaderStageBinary& source);

        static uint GetStageType(ShaderStage stage);
    };
} // namespace Dodo::Platform
