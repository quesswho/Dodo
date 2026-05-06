// Shadow.slang - CSM depth pass using vertex instancing.
// Each mesh is drawn with instanceCount = numCascades. The VS selects
// the per-cascade light-space matrix via SV_InstanceID and outputs to
// correct render target via SV_RenderTargetArrayIndex.
// Requires the VK_KHR_shader_draw_parameters device extension for instancing
#include "../Common/Uniforms.slang"
#include "../Common/CsmUniforms.slang"

struct VertexInput
{
    float3 position;
};

struct VSOutput
{
    float4 position : SV_Position;
    uint   layer    : SV_RenderTargetArrayIndex;
};

[shader("vertex")]
VSOutput vertexMain(VertexInput input, uint instanceID : SV_InstanceID)
{
    float4 posWS = mul(u_Model, float4(input.position, 1.0f));
    VSOutput output;
    output.position = mul(u_LightSpaceMatrices[instanceID], posWS);
    output.layer    = instanceID;
    return output;
}
