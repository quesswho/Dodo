// ForwardLit.slang
#include "../Common/Uniforms.slang"
#include "../Common/BindlessUniforms.slang"
#include "../Common/Material.slang"
#include "../Common/VertexInterface.slang"
#include "../Common/TangentSpace.slang"
#include "../Lightning/ShadowSampling.slang"
#include "../Lightning/CookTorrance.slang"
#include "../Lightning/Lights.slang"

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.positionSS = mul(u_Camera, float4(output.positionWS, 1.0f));
    output.texCoord   = input.texCoord;
    output.normalWS   = normalize(mul(u_NormalMatrix, float4(input.normal,  0.0f)).xyz);
    output.tangentWS  = float4(normalize(mul(u_NormalMatrix, float4(input.tangent.xyz, 0.0f)).xyz), input.tangent.w);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    SamplerState samp = g_Samplers[u_SamplerIdx];

    float4 albedoSample = g_Textures[NonUniformResourceIndex(u_AlbedoIdx)].Sample(samp, input.texCoord);

    if (albedoSample.a < 0.5f) discard;

    // Reconstruct world-space TBN for normal mapping
    float3x3 TBN = buildTBN(input.normalWS, input.tangentWS);

    float3 normalTS = g_Textures[NonUniformResourceIndex(u_NormalIdx)].Sample(samp, input.texCoord).rgb * 2.0f - 1.0f;
    float3 N = normalize(mul(normalTS, TBN));

    float roughness = g_Textures[NonUniformResourceIndex(u_RoughIdx)].Sample(samp, input.texCoord).g;
    float metallic  = g_Textures[NonUniformResourceIndex(u_MetallicIdx)].Sample(samp, input.texCoord).b;
    float aoSample  = g_Textures[NonUniformResourceIndex(u_AoIdx)].Sample(samp, input.texCoord).r;
    float ao        = (aoSample > 0.0f) ? aoSample : 1.0f;

    float3 V = normalize(u_CameraPos - input.positionWS);

    DirectionalLight dl;
    dl.m_Color     = float3(1.0f, 1.0f, 0.87f) * 5.0f;
    dl.m_Direction = u_LightDir;

    float3 radiance = dl.Irradiance(input.positionWS);
    float3 L        = normalize(-dl.m_Direction);

    float viewDepth = abs(mul(u_CameraView, float4(input.positionWS, 1.0f)).z);
    float shadow    = shadowCalculationCSMPCSS(input.positionWS, N, L, g_ShadowMap,
                                               g_Samplers[SAMPLER_SHADOW], viewDepth, 10.0f);
    float3 Lo      = evaluateBRDFDirect(radiance, albedoSample.rgb, N, V, L, roughness, metallic, shadow);

    float3 F0         = lerp(float3(0.01f, 0.01f, 0.01f), albedoSample.rgb, metallic);
    float  NdotV      = max(dot(N, V), 0.001f);
    float3 irradiance = g_IrradianceMap.SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], N, 0).rgb;

    float3 ambient = evaluateAmbient(albedoSample.rgb, ao, metallic, roughness, NdotV, F0, irradiance);

    return float4(Lo + ambient * 0.1, albedoSample.a);
}
