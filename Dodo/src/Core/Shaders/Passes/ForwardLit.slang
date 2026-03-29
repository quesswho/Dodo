#include "../Common/Uniforms.slang"
#include "../Common/Material.slang"
#include "../Common/VertexInterface.slang"
#include "../Common/TangentSpace.slang"
#include "../Lightning/ShadowSampling.slang"
#include "../Lightning/BlinnPhong.slang"

// TODO: These bindings should be replaced by modern material structs, see https://shader-slang.org/docs/shader-cursors/

// Material textures
[[vk::binding(0, 1)]] Sampler2D<float4> u_DiffuseMap : register(t0) : register(s0);
[[vk::binding(1, 1)]] Sampler2D<float4> u_SpecularMap : register(t1) : register(s1);
[[vk::binding(2, 1)]] Sampler2D<float4> u_NormalMap : register(t2) : register(s2);

// Shadow map binding
[[vk::binding(3, 1)]] Sampler2D<float> u_ShadowMap : register(t3) : register(s3);

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.positionSS = mul(u_Camera, float4(output.positionWS, 1.0f));
    output.texCoord = input.texCoord;

    float3 T = normalize(mul(u_NormalMatrix, float4(input.tangent, 0.0f)).xyz);
    float3 N = normalize(mul(u_NormalMatrix, float4(input.normal, 0.0f)).xyz);
    float3x3 TBN = buildTBN(N, T);
    output.tangentCameraPos = mul(TBN, u_CameraPos);
    output.tangentFragPos = mul(TBN, output.positionWS);
    output.lightDirection = mul(TBN, normalize(-u_LightDir));
    output.normalWS = N;

    // Calculate light space position
    output.positionLL = mul(u_LightCamera, float4(output.positionWS, 1.0f));

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float4 albedo = u_DiffuseMap.Sample(input.texCoord.xy);

    // Early alpha discard
    if (albedo.a < 0.1f) discard;

    float3 normal = normalize(u_NormalMap.Sample(input.texCoord.xy).rgb * 2.0f - 1.0f);
    float specular = u_SpecularMap.Sample(input.texCoord.xy).r;

    // Compute shadow factor
    float shadow = shadowCalculationPCF(input.positionLL, normal, input.lightDirection, u_ShadowMap);

    // Evaluate Blinn-Phong lighting model with shadow factor
    float3 litColor = evaluateBlinnPhong(albedo.xyz, normal, specular, shadow, input.lightDirection, input.tangentCameraPos - input.tangentFragPos);
    
    float4 result = float4(litColor, albedo.a);

    return result;
}