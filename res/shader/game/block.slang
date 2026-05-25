#include "../builtin/Common/Uniforms.slang"
#include "../builtin/Common/BindlessUniforms.slang"

struct VertexInput {
    float3 position;
    float2 texCoord;
    float3 normal;
    float  texIdx;
};

struct VertexOutput {
    float4 positionSS : SV_Position;
    float3 positionWS;
    float2 texCoord;
    float3 normalWS;
    nointerpolation uint texIdx;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.positionSS = mul(u_Camera, float4(output.positionWS, 1.0f));
    output.texCoord   = input.texCoord;
    output.normalWS   = normalize(mul(u_NormalMatrix, float4(input.normal, 0.0f)).xyz);
    output.texIdx     = (uint)input.texIdx;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    SamplerState samp = g_Samplers[u_SamplerIdx];
    float3 color = g_Textures[NonUniformResourceIndex(input.texIdx)].Sample(samp, input.texCoord).rgb;
    float diff = max(dot(normalize(-u_LightDir), input.normalWS), 0.0f);
    return float4(color * 0.4f + color * diff * 0.8f, 1.0f);
}
