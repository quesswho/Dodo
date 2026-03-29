#pragma once
#include <Core/Common.h>

namespace Dodo {
    enum class MaterialFeatures : uint32_t {
        None = 0,
        AlbedoMap = 1 << 0,
        RoughnessMap = 1 << 1,
        MetallicMap = 1 << 2,
        NormalMap = 1 << 3,
        SpecularMap = 1 << 4,
        HeightMap = 1 << 5,
        EmissiveMap = 1 << 6,
        AoMap = 1 << 7
    };

    inline MaterialFeatures operator|(MaterialFeatures a, MaterialFeatures b) {
        return static_cast<MaterialFeatures>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline MaterialFeatures& operator|=(MaterialFeatures& a, MaterialFeatures b) {
        a = a | b;
        return a;
    }
    
    inline bool HasFeature(MaterialFeatures features, MaterialFeatures flag) {
        return (static_cast<uint32_t>(features) & static_cast<uint32_t>(flag)) != 0;
    }
} // namespace Dodo