// Shadow.slang
#include "../Common/Uniforms.slang"

struct VertexInput
{
    float3 position;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

// Shadow pass only needs to output depth, so we can skip pixel stage
[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(u_LightCamera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}
