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
        SampledTextureArray,
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

    enum class PushConstantMemberType {
        Float,
        Int,
        UInt
    };

    struct PushConstantMember {
        std::string name;
        uint32_t offset;       // byte offset within the raw data blob
        uint32_t elementCount; // 1 = scalar, 2 = vec2, 3 = vec3, 4 = vec4
        PushConstantMemberType scalarType;
    };

    struct PushConstantReflection {
        std::string instanceName;
        std::vector<PushConstantMember> members;
        bool hasPushConstant = false;
    };

    struct VertexInputAttributeReflection {
        uint32_t location;
        uint32_t componentCount; // 1-4, scalar type assumed to be float
    };

    /**
     * Slang shader owning compiled backend specific binaries and reflection data
     */
    struct ShaderAsset {
        std::string path;

        std::vector<ShaderStageBinary> stages;
        std::vector<DescriptorBindingReflection> descriptorBindings;
        std::vector<VertexInputAttributeReflection> vertexInputs;
        PushConstantReflection pushConstant;
    };
} // namespace Dodo
