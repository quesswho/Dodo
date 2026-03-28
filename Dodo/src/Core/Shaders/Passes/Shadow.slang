#include "../Common/Uniforms.slang"

struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(u_LightCamera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}