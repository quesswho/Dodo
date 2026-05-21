// Skybox.slang
#include "../Common/Uniforms.slang"
#include "../Common/BindlessUniforms.slang"

struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 positionSS : SV_Position;
    float3 positionWS;
    float3 texCoord;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.texCoord = input.position;
    float4 clipPos = mul(u_SkyboxCamera, float4(output.positionWS, 1.0f));
    output.positionSS = float4(clipPos.xy, clipPos.w, clipPos.w);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float4 result = g_EnvironmentMap.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], input.texCoord.xyz);
    return result;
}
