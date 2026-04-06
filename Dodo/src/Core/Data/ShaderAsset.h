#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Dodo {

    enum class ShaderStage {
        Unknown,
        Vertex,
        Fragment,
        Geometry,
        Compute
    };

    /**
     * Every different shader backend supported
     */
    enum class ShaderBackendTarget {
        VulkanSPIRV,
        OpenGLGLSL,
        // MetalMSL,
    };

    /**
     * Binary data for either SPIR-V or GLSL (not actually binary but it is the closest we can get to compiled code)
     */
    struct ShaderStageBinary {
        ShaderStage stage;
        std::string entryPoint;
        std::vector<uint32_t> spirv;
        std::string glsl; // (OpenGL)
    };

    enum class DescriptorType {
        UniformBuffer,
        SampledTexture,
        SampledCubeMap,
        Sampler,
        CombinedImageSampler, // Legacy, use separate Texture2D + SamplerState instead
    };

    struct DescriptorBindingReflection {
        uint32_t set;
        uint32_t binding;
        uint32_t count;
        DescriptorType type;
    };

    /**
     * Slang shader owning compiled backend specific binaries and reflection data
     */
    struct ShaderAsset {
        std::string path;
        std::string slangSource;

        std::vector<ShaderStageBinary> stages;
        std::vector<DescriptorBindingReflection> descriptorBindings;
    };
} // namespace Dodo
