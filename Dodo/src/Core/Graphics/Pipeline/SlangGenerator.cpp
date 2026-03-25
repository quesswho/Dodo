#include "SlangGenerator.h"

// Note: Whole file is based on the old GLSL shader, the conversion to slang was generated using AI, this is completely
// temporary and will be replaced by a more flexible generator

namespace {
    bool HasFlag(Dodo::ShaderBuilderFlags flags, Dodo::ShaderBuilderFlags bit)
    {
        return static_cast<uint32_t>(flags & bit) != 0;
    }
} // namespace

namespace Dodo {

    SlangSource SlangGenerator::GetFallbackShader()
    {
        SlangSource shader;
        shader.name = "GeneratedFallback";
        shader.source = "\n"
                        "[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)\n"
                        "{\n"
                        "    float4x4 u_Camera;\n"
                        "    float4x4 u_LightCamera;\n"
                        "    float3 u_LightDir;\n"
                        "    float u_FramePadding0;\n"
                        "    float3 u_CameraPos;\n"
                        "    float u_FramePadding1;\n"
                        "};\n"
                        "[[vk::binding(1, 0)]] cbuffer ModelData : register(b1)\n"
                        "{\n"
                        "    float4x4 u_Model;\n"
                        "    float4x4 u_NormalMatrix;\n"
                        "};\n"
                        "\n"
                        "struct VertexInput\n"
                        "{\n"
                        "    float3 position : POSITION;\n"
                        "    float2 texcoord : TEXCOORD0;\n"
                        "};\n"
                        "\n"
                        "struct VertexOutput\n"
                        "{\n"
                        "    float4 position : SV_Position;\n"
                        "    float2 texcoord : TEXCOORD0;\n"
                        "};\n"
                        "\n"
                        "[shader(\"vertex\")]\n"
                        "VertexOutput vertexMain(VertexInput input)\n"
                        "{\n"
                        "    VertexOutput output;\n"
                        "    output.texcoord = input.texcoord;\n"
                        "    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));\n"
                        "    return output;\n"
                        "}\n"
                        "\n"
                        "float checker(float2 uv)\n"
                        "{\n"
                        "    float cx = floor(20.0f * uv.x);\n"
                        "    float cy = floor(20.0f * uv.y);\n"
                        "    return sign(fmod(cx + cy, 2.0f));\n"
                        "}\n"
                        "\n"
                        "[shader(\"fragment\")]\n"
                        "float4 fragmentMain(VertexOutput input) : SV_Target\n"
                        "{\n"
                        "    float res = lerp(1.0f, 0.0f, checker(input.texcoord));\n"
                        "    return float4(res, res * 0.1f, res * 0.1f, 1.0f);\n"
                        "}\n";
        return shader;
    }

    SlangSource SlangGenerator::Generate(ShaderBuilderFlags flags)
    {
        const bool noTexcoord = HasFlag(flags, ShaderBuilderFlagNoTexcoord);
        const bool useCubeMap = HasFlag(flags, ShaderBuilderFlagCubeMap);
        const bool useNormalMap = HasFlag(flags, ShaderBuilderFlagNormalMap);
        const bool useTangentSpace = HasFlag(flags, ShaderBuilderFlagTangentSpace) || useNormalMap;
        const bool useColorUniform = HasFlag(flags, ShaderBuilderFlagColorUniform);
        const bool useCameraPos = HasFlag(flags, ShaderBuilderFlagsCameraPositionUniform) ||
                                  HasFlag(flags, ShaderBuilderFlagSpecularUniform) ||
                                  HasFlag(flags, ShaderBuilderFlagDiffuseMap) ||
                                  HasFlag(flags, ShaderBuilderFlagSpecularMap) || useNormalMap;
        const bool useLight = HasFlag(flags, ShaderBuilderFlagLightDirectionUniform) ||
                              HasFlag(flags, ShaderBuilderFlagSpecularUniform) ||
                              HasFlag(flags, ShaderBuilderFlagDiffuseMap) ||
                              HasFlag(flags, ShaderBuilderFlagSpecularMap) || useNormalMap;
        const bool useShadowMap = HasFlag(flags, ShaderBuilderFlagShadowMap);
        const bool useBasicTexture = HasFlag(flags, ShaderBuilderFlagBasicTexture);
        const bool useDiffuseMap = HasFlag(flags, ShaderBuilderFlagDiffuseMap);
        const bool useSpecularMap = HasFlag(flags, ShaderBuilderFlagSpecularMap);
        const bool useSpecularUniform = HasFlag(flags, ShaderBuilderFlagSpecularUniform);
        const bool useMaxDepth = HasFlag(flags, ShaderBuilderFlagMaxDepth);

        std::string source;
        source.reserve(8192);

        source.append("[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)\n"
                      "{\n"
                      "    float4x4 u_Camera;\n"
                      "    float4x4 u_LightCamera;\n"
                      "    float3 u_LightDir;\n"
                      "    float u_FramePadding0;\n"
                      "    float3 u_CameraPos;\n"
                      "    float u_FramePadding1;\n"
                      "};\n"
                      "[[vk::binding(1, 0)]] cbuffer ModelData : register(b1)\n"
                      "{\n"
                      "    float4x4 u_Model;\n"
                      "    float4x4 u_NormalMatrix;\n"
                      "};\n"
                      "\n"
                      "\n");

        if (useColorUniform)
            source.append(
                "[[vk::binding(2, 0)]] cbuffer MaterialColor : register(b2) { float3 u_Color; float u_ColorPadding; "
                "};\n");
        if (useSpecularUniform)
            source.append("[[vk::binding(3, 0)]] cbuffer MaterialSpecular : register(b3) { float u_Specular; float3 "
                          "u_SpecularPadding; };\n");

        if (useDiffuseMap) {
            source.append("[[vk::binding(0, 1)]] Sampler2D<float4> u_DiffuseMap : register(t0) : register(s0);\n");
        } else if (useCubeMap) {
            source.append("[[vk::binding(0, 1)]] SamplerCube<float4> u_CubeMap : register(t0) : register(s0);\n");
        } else if (useBasicTexture) {
            source.append("[[vk::binding(0, 1)]] Sampler2D<float4> u_TextureMap : register(t0) : register(s0);\n");
        }

        if (useSpecularMap) {
            source.append("[[vk::binding(1, 1)]] Sampler2D<float4> u_SpecularMap : register(t1) : register(s1);\n");
        }

        if (useNormalMap) {
            source.append("[[vk::binding(2, 1)]] Sampler2D<float4> u_NormalMap : register(t2) : register(s2);\n");
        }

        if (useShadowMap) {
            source.append("[[vk::binding(3, 1)]] Sampler2D<float> u_DepthMap : register(t3) : register(s3);\n");
        }

        source.append("\nstruct VertexInput\n{\n");
        source.append("    float3 position : POSITION;\n");

        if (!noTexcoord) {
            source.append(useCubeMap ? "    float3 texcoord : TEXCOORD0;\n" : "    float2 texcoord : TEXCOORD0;\n");
        }

        if (useTangentSpace) {
            source.append("    float3 normal : NORMAL;\n");
            source.append("    float3 tangent : TANGENT;\n");
        } else if (!useCubeMap || !noTexcoord) {
            source.append("    float3 normal : NORMAL;\n");
        }

        source.append("};\n\n");

        source.append("struct VertexOutput\n{\n");
        source.append("    float4 position : SV_Position;\n");
        source.append("    float3 fragPos : TEXCOORD0;\n");

        if (!noTexcoord) {
            source.append(useCubeMap ? "    float3 texCoord : TEXCOORD1;\n" : "    float2 texCoord : TEXCOORD1;\n");
        } else if (useCubeMap) {
            source.append("    float3 texCoord : TEXCOORD1;\n");
        }

        if (useLight) source.append("    float3 lightDirection : TEXCOORD2;\n");

        if (useNormalMap) {
            source.append("    float3 tangentCameraPos : TEXCOORD3;\n");
            source.append("    float3 tangentFragPos : TEXCOORD4;\n");
        } else if (!useCubeMap) {
            source.append("    float3 normal : TEXCOORD3;\n");
        }

        if (useCameraPos) source.append("    float3 cameraPos : TEXCOORD5;\n");
        if (useShadowMap) source.append("    float4 lightFragPos : TEXCOORD6;\n");
        source.append("};\n\n");

        if (useShadowMap) {
            source.append("float ShadowCalculation(float4 lightFragPos, float3 normal, float3 lightDirection)\n"
                          "{\n"
                          "    float3 projCoords = lightFragPos.xyz / lightFragPos.w;\n"
                          "    if (projCoords.z > 1.0f)\n"
                          "        return 0.0f;\n"
                          "    projCoords = projCoords * 0.5f + 0.5f;\n"
                          "    float currentDepth = projCoords.z;\n"
                          "    float bias = max(0.005f * (1.0f - dot(normal, lightDirection)), 0.001f);\n"
                          "    uint depthWidth = 0;\n"
                          "    uint depthHeight = 0;\n"
                          "    u_DepthMap.GetDimensions(depthWidth, depthHeight);\n"
                          "    float2 texelSize = 1.0f / float2(depthWidth, depthHeight);\n"
                          "    float shadow = 0.0f;\n"
                          "    const int kernelRadius = 1;\n"
                          "    for (int x = -kernelRadius; x <= kernelRadius; ++x)\n"
                          "    {\n"
                          "        for (int y = -kernelRadius; y <= kernelRadius; ++y)\n"
                          "        {\n"
                          "            float closestDepth = u_DepthMap.Sample(projCoords.xy + float2(x, y) * "
                          "texelSize);\n"
                          "            if (currentDepth - bias > closestDepth)\n"
                          "                shadow += 1.0f;\n"
                          "        }\n"
                          "    }\n"
                          "    float samples = kernelRadius*2 + 1;\n"
                          "    return shadow / (samples*samples);\n"
                          "}\n\n");
        }

        source.append("[shader(\"vertex\")]\n");
        source.append("VertexOutput vertexMain(VertexInput input)\n{\n");
        source.append("    VertexOutput output;\n");
        source.append("    output.fragPos = mul(u_Model, float4(input.position, 1.0f)).xyz;\n");

        if (!noTexcoord) {
            source.append("    output.texCoord = input.texcoord;\n");
        } else if (useCubeMap) {
            source.append("    output.texCoord = input.position;\n");
        }

        if (useLight && !useNormalMap) {
            source.append("    output.lightDirection = normalize(-u_LightDir);\n");
        }

        if (useNormalMap) {
            source.append("    float3 T = normalize(mul(u_NormalMatrix, float4(input.tangent, 0.0f)).xyz);\n"
                          "    float3 N = normalize(mul(u_NormalMatrix, float4(input.normal, 0.0f)).xyz);\n"
                          "    T = normalize(T - dot(T, N) * N);\n"
                          "    float3 B = cross(N, T);\n"
                          "    float3x3 TBN = transpose(float3x3(T, B, N));\n"
                          "    output.tangentCameraPos = mul(TBN, u_CameraPos);\n"
                          "    output.tangentFragPos = mul(TBN, output.fragPos);\n"
                          "    output.lightDirection = mul(TBN, normalize(-u_LightDir));\n");
        } else if (!useCubeMap) {
            source.append("    output.normal = normalize(mul(u_NormalMatrix, float4(input.normal, 0.0f)).xyz);\n");
        }

        if (useCameraPos) source.append("    output.cameraPos = u_CameraPos;\n");
        if (useShadowMap)
            source.append(
                "    output.lightFragPos = mul(u_LightCamera, mul(u_Model, float4(input.position, 1.0f)));\n");

        if (useMaxDepth) {
            source.append("    float4 clipPos = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));\n"
                          "    output.position = float4(clipPos.xy, clipPos.w, clipPos.w);\n");
        } else {
            source.append("    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));\n");
        }

        source.append("    return output;\n}\n\n");

        source.append("[shader(\"fragment\")]\n");
        source.append("float4 fragmentMain(VertexOutput input) : SV_Target\n{\n");
        source.append("    float4 result = float4(0.0f, 0.0f, 0.0f, 1.0f);\n");

        if (useCubeMap) source.append("    result = u_CubeMap.Sample(input.texCoord.xyz);\n");

        if (useNormalMap) {
            source.append("    float3 normal = normalize(u_NormalMap.Sample(input.texCoord.xy).rgb * 2.0f - 1.0f);\n");
        } else if (!useCubeMap) {
            source.append("    float3 normal = normalize(input.normal);\n");
        }

        if (useDiffuseMap) {
            if (useColorUniform) {
                source.append("    float4 color = u_DiffuseMap.Sample(input.texCoord.xy) * float4(u_Color, 1.0f);\n");
            } else {
                source.append("    float4 color = u_DiffuseMap.Sample(input.texCoord.xy);\n");
            }
            source.append("    if (color.a < 0.85f) discard;\n");
        } else if (!useCubeMap) {
            if (useColorUniform) {
                source.append("    float4 color = float4(u_Color, 1.0f);\n");
            } else if (useBasicTexture) {
                source.append("    float4 color = u_TextureMap.Sample(input.texCoord.xy);\n");
            } else {
                source.append("    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);\n");
            }
        }

        if (useShadowMap) {
            source.append("    float shadow = ShadowCalculation(input.lightFragPos, normal, input.lightDirection);\n");
        } else {
            source.append("    float shadow = 0.0f;\n");
        }

        if (useLight) {
            source.append("    float3 ambient = color.rgb * 0.1f;\n");
            source.append(
                "    float4 diffuse = float4(max(dot(input.lightDirection, normal), 0.0f) * color.rgb, color.a);\n");

            if (useNormalMap) {
                source.append("    float3 viewDir = normalize(input.tangentCameraPos - input.tangentFragPos);\n");
            } else {
                source.append("    float3 viewDir = normalize(input.cameraPos - input.fragPos);\n");
            }

            source.append("    float3 halfwayDir = normalize(input.lightDirection + viewDir);\n");

            if (useSpecularUniform) {
                source.append("    float3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * u_Specular;\n");
                source.append(
                    "    result = float4(ambient + (1.0f - shadow) * (diffuse.rgb + specular), diffuse.a);\n");
            } else if (useSpecularMap) {
                source.append("    float3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * "
                              "u_SpecularMap.Sample(input.texCoord.xy).rrr;\n");
                source.append(
                    "    result = float4(ambient + (1.0f - shadow) * (diffuse.rgb + specular), diffuse.a);\n");
            } else {
                source.append("    result = float4(ambient + (1.0f - shadow) * diffuse.rgb, diffuse.a);\n");
            }
        } else if (!useCubeMap) {
            source.append("    result = color;\n");
            source.append("    if (result.a < 0.1f) discard;\n");
        }

        source.append("    return result;\n}\n");

        SlangSource shader;
        shader.name = "GeneratedMaterial_" + std::to_string(static_cast<uint32_t>(flags));
        shader.source = std::move(source);
        return shader;
    }
} // namespace Dodo
