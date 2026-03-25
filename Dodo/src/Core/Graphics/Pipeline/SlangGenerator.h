#pragma once

#include <Core/Common.h>

#include "ShaderSource.h"

#include <string>
#include <type_traits>

namespace Dodo {

    enum ShaderBuilderFlags : uint32_t {
        ShaderBuilderFlagNone = 0,
        ShaderBuilderFlagNoTexcoord = 1 << 0,
        ShaderBuilderFlagTangentSpaceNormal = 1 << 1,
        ShaderBuilderFlagTangentSpace = 1 << 2,
        ShaderBuilderFlagColorUniform = 1 << 3,
        ShaderBuilderFlagsCameraPositionUniform = 1 << 4,
        ShaderBuilderFlagLightDirectionUniform = 1 << 5,
        ShaderBuilderFlagSpecularUniform = 1 << 6,
        ShaderBuilderFlagDiffuseMap = 1 << 7,
        ShaderBuilderFlagSpecularMap = 1 << 8,
        ShaderBuilderFlagNormalMap = 1 << 9,
        ShaderBuilderFlagCubeMap = 1 << 10,
        ShaderBuilderFlagMaxDepth = 1 << 11,
        ShaderBuilderFlagBasicTexture = 1 << 12,
        ShaderBuilderFlagShadowMap = 1 << 13,
    };

    inline ShaderBuilderFlags operator|(ShaderBuilderFlags a, ShaderBuilderFlags b) noexcept
    {
        using U = std::underlying_type_t<ShaderBuilderFlags>;
        return static_cast<ShaderBuilderFlags>(static_cast<U>(a) | static_cast<U>(b));
    }

    inline ShaderBuilderFlags operator&(ShaderBuilderFlags a, ShaderBuilderFlags b) noexcept
    {
        using U = std::underlying_type_t<ShaderBuilderFlags>;
        return static_cast<ShaderBuilderFlags>(static_cast<U>(a) & static_cast<U>(b));
    }

    inline ShaderBuilderFlags& operator|=(ShaderBuilderFlags& a, ShaderBuilderFlags b) noexcept
    {
        a = a | b;
        return a;
    }

    class SlangGenerator {
      public:
        static SlangSource Generate(ShaderBuilderFlags flags);
        static SlangSource GetFallbackShader();
    };

    using ShaderGenerator = SlangGenerator;
} // namespace Dodo
