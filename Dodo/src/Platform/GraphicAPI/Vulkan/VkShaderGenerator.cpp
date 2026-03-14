#include "VkShaderGenerator.h"
#include "pch.h"

namespace Dodo::Platform {

    // TODO: Replace with slang
    GeneratedShaderSource VkShaderGenerator::GetFallbackShader() {
        GeneratedShaderSource source;
        return source;
    }

    GeneratedShaderSource VkShaderGenerator::Generate(const ShaderBuilderFlags flags) 
    {
        GeneratedShaderSource source;
        return source;
    }
} // namespace Dodo::Platform