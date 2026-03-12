#pragma once

#include <string>

#include "ShaderSource.h"

#include <slang.h>
#include <slang-com-ptr.h>

namespace Dodo {
    
    class SlangCompiler {
    public:
        enum class Target {
            GLSL,
            SPIRV,
        };
        SlangCompiler(Target target);  // Init happens in constructor
        ~SlangCompiler(); // Shutdown happens in destructor

        ShaderSource CompileFile(const std::string& path);
        ShaderSource CompileFromString(const std::string& source, const std::string& name);
    private:
        ShaderSource CompileModule(slang::IModule* module);

        Slang::ComPtr<slang::IGlobalSession> m_GlobalSession;
        Slang::ComPtr<slang::ISession>       m_Session;
    };
}