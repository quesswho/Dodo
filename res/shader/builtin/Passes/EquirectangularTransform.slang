// EquirectangularTransform.slang
// This shader takes an equirectangular image and projects it on a cube.
// For each face capture, u_Camera = projection * lookat view, u_Model = identity.
#include "../Common/Uniforms.slang"

[[vk::binding(0, 1)]] Texture2D<float4> textureMap : register(t0);
[[vk::binding(1, 1)]] SamplerState textureSampler : register(s0);

struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 localPos : TEXCOORD0;
};

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.localPos = input.position;
    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float2 uv = SampleSphericalMap(normalize(input.localPos));
    float3 color = textureMap.Sample(textureSampler, uv).rgb;
    return float4(color, 1.0f);
}
