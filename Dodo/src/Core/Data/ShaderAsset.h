#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Dodo {

    /**
     * Every different shader backend supported
     */
    enum class ShaderBackendTarget {
        VulkanSPIRV,
        OpenGLGLSL,
        //MetalMSL,
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

    /**
     * Slang shader owning compiled backend specific binaries
     */
    struct ShaderAsset {
        std::string path;
        std::string slangSource;

        std::vector<ShaderStageBinary> stages;
        // TODO: Reflection, layout maybe hash?
    };
} // namespace Dodo