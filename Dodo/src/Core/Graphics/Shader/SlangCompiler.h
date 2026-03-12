#pragma once

#include <string>

#include "ShaderSource.h"

namespace Dodo {
    
    class SlangCompiler
    {
    public:

        static ShaderSource Compile(const std::string& path);
    };
}