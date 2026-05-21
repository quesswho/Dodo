// Gamma.slang
#include "../Common/Uniforms.slang"
#include "../Common/BindlessUniforms.slang"

struct VertexInput
{
    float2 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.texCoord = input.texCoord;
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input, uniform float gamma, uniform float exposure) : SV_Target
{
    SamplerState samp = g_Samplers[u_SamplerIdx];
    float3 color = g_Textures[NonUniformResourceIndex(u_AlbedoIdx)].Sample(samp, input.texCoord).rgb;
    color *= exposure;
    float3 result = pow(color / (color + float3(1.0f, 1.0f, 1.0f)), float3(1.0f / gamma));
    return float4(result, 1.0f);
}
