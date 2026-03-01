#pragma once

#include <Core/Common.h>

#include "Core/Graphics/Shader/Shader.h"

namespace Dodo {

    enum ShaderBuilderFlags : uint32_t
    {
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

    // TEMPORARY //
    inline ShaderBuilderFlags operator|(ShaderBuilderFlags a, ShaderBuilderFlags b) noexcept
    {
        using U = std::underlying_type_t<ShaderBuilderFlags>;
        return static_cast<ShaderBuilderFlags>(static_cast<U>(a) | static_cast<U>(b));
    }

    inline ShaderBuilderFlags &operator|=(ShaderBuilderFlags &a, ShaderBuilderFlags b) noexcept
    {
        a = a | b;
        return a;
    }

    namespace Platform {

        class OpenGLShaderBuilder {
            Ref<Shader> m_FallbackShader;

          public:
            OpenGLShaderBuilder();
            ~OpenGLShaderBuilder();

            Ref<Shader> BuildVertexFragmentShader(const ShaderBuilderFlags flags, const char *name) const;
            inline Ref<Shader> BuildVertexFragmentShader(const ShaderBuilderFlags flags) const
            {
                return BuildVertexFragmentShader(flags, std::to_string(flags).c_str());
            }

            uint CompileVertexFragmentShader(const char *vertex, const char *fragment) const;

            inline Ref<Shader> GetFallbackShader() const { return m_FallbackShader; }

          private:
            void InitFallbackShader();
        };
    } // namespace Platform
} // namespace Dodo