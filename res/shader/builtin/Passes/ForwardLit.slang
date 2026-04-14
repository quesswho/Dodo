#include "../Common/Uniforms.slang"
#include "../Common/Material.slang"
#include "../Common/VertexInterface.slang"
#include "../Common/TangentSpace.slang"
#include "../Lightning/ShadowSampling.slang"
#include "../Lightning/CookTorrance.slang"

// Metallic/roughness PBR texture set (set 1)
[[vk::binding(0, 1)]] Texture2D<float4> u_AlbedoMap;
[[vk::binding(1, 1)]] Texture2D<float4> u_RoughnessMap; // sample .g (works for both ORM-packed and greyscale)
[[vk::binding(2, 1)]] Texture2D<float4> u_NormalMap;
[[vk::binding(3, 1)]] Texture2D<float>  u_ShadowMap;

// Shared sampler for all material textures and shadow map
[[vk::binding(4, 1)]] SamplerState u_LinearSampler;

[[vk::binding(5, 1)]] Texture2D<float4> u_MetallicMap; // sample .b (works for both ORM-packed and greyscale)
[[vk::binding(6, 1)]] Texture2D<float4> u_AoMap;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.positionSS = mul(u_Camera, float4(output.positionWS, 1.0f));
    output.texCoord   = input.texCoord;
    output.normalWS   = normalize(mul(u_NormalMatrix, float4(input.normal,  0.0f)).xyz);
    output.tangentWS  = normalize(mul(u_NormalMatrix, float4(input.tangent, 0.0f)).xyz);
    output.positionLL = mul(u_LightCamera, float4(output.positionWS, 1.0f));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float4 albedoSample = u_AlbedoMap.Sample(u_LinearSampler, input.texCoord);

    // Keep threshold near zero so semi-transparent regions reach the blender
    if (albedoSample.a < 0.001f) discard;

    // Reconstruct world-space TBN for normal mapping
    float3x3 TBN = buildTBN(input.normalWS, input.tangentWS);

    // Transform normal from tangent space to world space:
    //   mul(v, M) = v * M  =>  [v.x*T + v.y*B + v.z*N]  (correct tangent-to-world)
    float3 normalTS = u_NormalMap.Sample(u_LinearSampler, input.texCoord).rgb * 2.0f - 1.0f;
    float3 N = normalize(mul(normalTS, TBN));

    float roughness = u_RoughnessMap.Sample(u_LinearSampler, input.texCoord).g;
    float metallic  = u_MetallicMap.Sample(u_LinearSampler, input.texCoord).b;
    // Default ao to 1.0 so ambient isn't zeroed out when no AO map is bound
    float aoSample  = u_AoMap.Sample(u_LinearSampler, input.texCoord).r;
    float ao        = (aoSample > 0.0f) ? aoSample : 1.0f;

    float3 V = normalize(u_CameraPos - input.positionWS);
    float3 L = normalize(-u_LightDir);

    float shadow = shadowCalculationPCF(input.positionLL, N, L, u_ShadowMap, u_LinearSampler);

    float3 litColor = evaluateCookTorrance(albedoSample.rgb, N, V, L, roughness, metallic, ao, shadow);

    return float4(litColor, albedoSample.a);
}
