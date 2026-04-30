// CsmUniforms.slang
// Cascaded shadow map light space matrices and cascade split depths.
// Uploaded once per frame by Renderer3D before the shadow and scene passes.

static const int CSM_NUM_CASCADES = 4;

[[vk::binding(3, 0)]] cbuffer LightSpaceData : register(b3)
{
    float4x4 u_LightSpaceMatrices[CSM_NUM_CASCADES];
    float4   u_CascadeSplitDepths; // x,y,z,w = view-space depth of each cascade far plane
    int      u_NumCascades;
};
