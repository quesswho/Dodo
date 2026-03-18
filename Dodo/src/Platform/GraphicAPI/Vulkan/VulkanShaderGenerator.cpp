#include "VulkanShaderGenerator.h"
#include "pch.h"

namespace Dodo::Platform {

    // TODO: Replace with slang
    GeneratedShaderSource VulkanShaderGenerator::GetFallbackShader()
    {
        GeneratedShaderSource source;
        return source;
    }

    GeneratedShaderSource VulkanShaderGenerator::Generate(const ShaderBuilderFlags flags)
    {
        GeneratedShaderSource source;
        return source;
    }
} // namespace Dodo::Platform