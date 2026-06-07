#pragma once

#include <string>
#include <vector>

#include "Core/Data/ShaderAsset.h"

#include <slang-com-ptr.h>
#include <slang.h>

namespace Dodo {

    class SlangCompiler {
      public:
        SlangCompiler();
        ~SlangCompiler();

        ShaderAsset CompileFile(const std::string& path);
        ShaderAsset CompileFromString(const std::string& source, const std::string& name);

      private:
        ShaderAsset CompileModule(slang::IModule* module, const std::string& path);
        ShaderStageBinary CompileEntryPoint(slang::IModule* module, slang::IEntryPoint* entryPoint);
        static ShaderStage GetShaderStage(SlangStage stage);
        static std::vector<uint32_t> BlobToSPIRV(slang::IBlob* code);

        Slang::ComPtr<slang::IGlobalSession> m_GlobalSession;
        Slang::ComPtr<slang::ISession> m_Session;
    };
} // namespace Dodo
