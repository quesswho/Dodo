#include "VkShaderCompiler.h"
#include <Core/Utilities/Logger.h>
#include <glad/gl.h>

#include <regex>

namespace Dodo::Platform {

    uint VkShaderCompiler::Compile(const ShaderSource& source)
    {
        return 0;
    }

    uint VkShaderCompiler::CompileStage(GLenum type, const std::string& source)
    {
        return 0;
    }
} // namespace Dodo::Platform
