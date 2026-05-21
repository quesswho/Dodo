#pragma once

#include <cstdint>

namespace Dodo::Platform {

    struct GPUModelData {
        float    model[16];
        float    normal[16];
        uint32_t albedoIdx;
        uint32_t roughIdx;
        uint32_t normalIdx;
        uint32_t metallicIdx;
        uint32_t aoIdx;
        uint32_t specIdx;
        uint32_t samplerIdx;
        uint32_t pad;
    };

} // namespace Dodo::Platform
