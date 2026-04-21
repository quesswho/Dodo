// Fallback.slang
// This shader is used for drawing an error texture when there was no material found for a given mesh.
#include "Uniforms.slang"

struct VertexInput
{
    float3 position;
    float2 texCoord;
};

struct VertexOutput
{
    float4 positionSS : SV_Position;
    float2 texCoord;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.texCoord = input.texCoord;
    output.positionSS = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

float checker(float2 uv)
{
    float cx = floor(20.0f * uv.x);
    float cy = floor(20.0f * uv.y);
    return sign(fmod(cx + cy, 2.0f));
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float res = lerp(1.0f, 0.0f, checker(input.texCoord));
    return float4(res, res * 0.1f, res * 0.1f, 1.0f);
}
