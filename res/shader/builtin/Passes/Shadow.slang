// Shadow.slang
// Cascaded shadow map depth pass.
// The geometry shader (instanced CSM_NUM_CASCADES times) fans each triangle out to all
// cascade layers in a single draw call, writing depth via gl_Layer / SV_RenderTargetArrayIndex.
#include "../Common/Uniforms.slang"
#include "../Common/CsmUniforms.slang"

struct VertexInput
{
    float3 position;
};

// Vertex shader outputs world-space position; the geometry shader applies per-cascade light transforms.
struct VSOutput
{
    float4 positionWS;
};

struct GSOutput
{
    float4 position : SV_Position;
    uint   layer    : SV_RenderTargetArrayIndex;
};

[shader("vertex")]
VSOutput vertexMain(VertexInput input)
{
    VSOutput output;
    output.positionWS = mul(u_Model, float4(input.position, 1.0f));
    return output;
}


[shader("geometry")]
[maxvertexcount(3)]
[instance(4)]
void geometryMain(
    triangle VSOutput input[3],
    uint invID : SV_GSInstanceID,
    inout TriangleStream<GSOutput> stream)
{
    for (int i = 0; i < 3; i++)
    {
        GSOutput output;
        output.position = mul(u_LightSpaceMatrices[invID], input[i].positionWS);
        output.layer    = invID;
        stream.Append(output);
    }
    stream.RestartStrip();
}
