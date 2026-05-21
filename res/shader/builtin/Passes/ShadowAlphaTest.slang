// ShadowAlphaTest.slang - CSM depth pass with alpha testing for cutout geometry.
// Like Shadow.slang but reads albedo alpha and discards fragments below threshold,
// producing correct silhouette shadows for foliage and other alpha-cutout meshes.
// Used when a material's pipeline has BlendMode::AlphaCutout.
#include "../Common/Uniforms.slang"
#include "../Common/BindlessUniforms.slang"
#include "../Common/CsmUniforms.slang"

struct VertexInput
{
    float3 position; // location 0
    float2 texCoord; // location 1
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord;
    uint   layer    : SV_RenderTargetArrayIndex;
};

[shader("vertex")]
VSOutput vertexMain(VertexInput input, uint instanceID : SV_InstanceID)
{
    float4 posWS = mul(u_Model, float4(input.position, 1.0f));
    VSOutput output;
    output.position = mul(u_LightSpaceMatrices[instanceID], posWS);
    output.texCoord = input.texCoord;
    output.layer    = instanceID;
    return output;
}

[shader("fragment")]
void fragmentMain(VSOutput input)
{
    float alpha = g_Textures[NonUniformResourceIndex(u_AlbedoIdx)].Sample(
        g_Samplers[u_SamplerIdx], input.texCoord).a;
    if (alpha < 0.5f) discard;
}
