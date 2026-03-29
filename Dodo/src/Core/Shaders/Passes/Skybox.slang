// Skybox.slang
#include "../Common/Uniforms.slang"

[[vk::binding(0, 1)]] SamplerCube<float4> cubeMapTexture : register(t0) : register(s0);

struct VertexInput
{
    float3 position;
    float3 texCoord;
};

// Output from vertex shader
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
    float4 clipPos = mul(u_Camera, float4(output.positionWS, 1.0f));
    output.positionSS = float4(clipPos.xy, clipPos.w, clipPos.w);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float4 result = cubeMapTexture.Sample(input.texCoord.xyz);
    return result;
}